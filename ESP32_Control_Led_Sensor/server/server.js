// server.js
const express = require('express');
const bodyParser = require('body-parser');
const mqtt = require('mqtt');
const path = require('path');

// --- Cấu hình Server ---
const app = express();
const PORT = 5000;

// --- Cấu hình MQTT ---
const MQTT_BROKER = "mqtt://broker.hivemq.com";
const MQTT_TOPIC_CONTROL = "home/led/control"; // Gửi lệnh
const MQTT_TOPIC_DATA = "home/sensor/data";   // Nhận dữ liệu

// Kết nối MQTT Broker
const client = mqtt.connect(MQTT_BROKER);

// Trạng thái hiện tại
let controlMode = "MANUAL"; // "MANUAL" hoặc "AUTO"
let currentLedState = "OFF";
let sensorData = { temperature: 0, humidity: 0, light: 0 }; // Dữ liệu cảm biến mới nhất

// Ngưỡng ánh sáng tự động (Giá trị analog LDR. Cần điều chỉnh theo môi trường!)
const LIGHT_THRESHOLD = 1500; 

// --- Cấu hình Express ---
app.use(bodyParser.json());
app.use(express.static(path.join(__dirname, 'public')));
app.set('view engine', 'ejs');
app.set('views', path.join(__dirname, 'views'));

// --- Logic MQTT ---
client.on('connect', () => {
    console.log(`Connected to MQTT Broker: ${MQTT_BROKER}`);
    // Đăng ký nhận dữ liệu cảm biến từ ESP32
    client.subscribe(MQTT_TOPIC_DATA, (err) => {
        if (!err) {
            console.log(`Subscribed to topic: ${MQTT_TOPIC_DATA}`);
        }
    });
});

client.on('message', (topic, message) => {
    // Xử lý dữ liệu cảm biến nhận được
    if (topic === MQTT_TOPIC_DATA) {
        try {
            const data = JSON.parse(message.toString());
            sensorData = data;
            console.log('Sensor Data received:', sensorData);

            // Kích hoạt logic tự động ngay khi có dữ liệu mới (chỉ khi ở chế độ AUTO)
            if (controlMode === "AUTO") {
                checkAutoControl();
            }

        } catch (e) {
            console.error('Error parsing MQTT JSON:', e);
        }
    }
});

// --- Hàm Gửi lệnh MQTT ---
function sendMqttCommand(command) {
    if (command === "ON" || command === "OFF") {
        client.publish(MQTT_TOPIC_CONTROL, command, (err) => {
            if (err) {
                console.error(`Failed to publish MQTT message: ${err}`);
            } else {
                console.log(`MQTT Published successfully: ${command}`);
                currentLedState = command;
            }
        });
    }
}

// --- Logic Tự động dựa trên Cảm biến LDR ---
function checkAutoControl() {
    const lightValue = sensorData.light;

    if (lightValue !== 0) { // Đảm bảo có dữ liệu
        // Trời tối (giá trị LDR thấp) -> BẬT LED
        if (lightValue < LIGHT_THRESHOLD && currentLedState !== "ON") {
            console.log(`AUTO MODE: Light (${lightValue}) < Threshold. Turning ON LED.`);
            sendMqttCommand("ON");
        } 
        // Trời sáng (giá trị LDR cao) -> TẮT LED
        else if (lightValue >= LIGHT_THRESHOLD && currentLedState !== "OFF") {
            console.log(`AUTO MODE: Light (${lightValue}) >= Threshold. Turning OFF LED.`);
            sendMqttCommand("OFF");
        }
    } else {
        console.log("Waiting for initial sensor data...");
    }
}


// --- Web Routes (API) ---

app.get('/', (req, res) => {
    // Truyền trạng thái và dữ liệu cảm biến sang template
    res.render('index', { 
        mode: controlMode, 
        led_state: currentLedState, 
        sensor: sensorData 
    });
});

app.post('/control', (req, res) => {
    const { mode, command } = req.body;
    
    if (mode === "AUTO") {
        controlMode = "AUTO";
        console.log("Control mode set to AUTO.");
        // Kích hoạt kiểm tra tự động ngay lập tức
        checkAutoControl(); 
        return res.json({ status: "success", message: "Mode set to AUTO. Server will handle light control based on LDR sensor." });
    }
    
    if (mode === "MANUAL" && (command === "ON" || command === "OFF")) {
        controlMode = "MANUAL";
        console.log(`Control mode set to MANUAL. Command: ${command}`);
        sendMqttCommand(command);
        return res.json({ status: "success", message: `LED set to ${command} manually.` });
    }
    
    if (mode === "GET_STATE") {
        return res.json({ status: "success", mode: controlMode, led_state: currentLedState, sensor: sensorData });
    }

    res.status(400).json({ status: "error", message: "Invalid command or mode." });
});


// Khởi chạy Server
app.listen(PORT, () => {
    console.log(`Node.js Server running on http://localhost:${PORT}`);
    console.log(`Accessable locally via: http://${require('ip').address()}:${PORT}`);
});