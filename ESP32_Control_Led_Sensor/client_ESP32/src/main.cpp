#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// --- Cấu hình Mạng & MQTT ---
const char* ssid = "OrsCorp";
const char* password = "Tamchiduc68";
const char* mqtt_server = "broker.hivemq.com"; // Broker công cộng
const char* mqtt_client_id = "ESP32_Smart_Client";

// Chủ đề (Topic)
const char* mqtt_topic_control = "home/led/control"; // Nhận lệnh
const char* mqtt_topic_data = "home/sensor/data";   // Gửi dữ liệu

// --- Cấu hình Cảm biến & LED ---
const int LED_PIN = 5;        // Chân LED (ví dụ: GPIO 2)
// const int LDR_PIN = 35;       // Chân Analog cho LDR (Chỉ dùng các chân có ADC - ví dụ: GPIO 32-39)
const int DHT_PIN = 4;       // Chân Digital cho DHT11
#define DHTTYPE DHT11         // Loại cảm biến

// Khai báo đối tượng
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHT_PIN, DHTTYPE);

unsigned long lastMsg = 0;
const long interval = 10000; // Gửi dữ liệu mỗi 5 giây

// --- Hàm khai báo ---
void setup_wifi();
void reconnect_mqtt();
void callback(char* topic, byte* payload, unsigned int length);
void sendSensorData();

// --- Các hàm đã cập nhật ---
void setup_wifi() { 
  delay(100);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// --- Hàm kết nối lại MQTT ---
void reconnect_mqtt() { 
  // Loop cho đến khi kết nối lại
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Cố gắng kết nối
    if (client.connect(mqtt_client_id)) {
      Serial.println("connected");
      // Đăng ký nhận lệnh từ Local Server
      client.subscribe(mqtt_topic_control);
      Serial.print("Subscribed to topic: ");
      Serial.println(mqtt_topic_control);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Chờ 5 giây trước khi thử lại
      delay(5000);
    }
  }
}

// --- Hàm xử lý lệnh MQTT nhận được ---
void callback(char* topic, byte* payload, unsigned int length) {  
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // So sánh lệnh và điều khiển LED
  if (strcmp(topic, mqtt_topic_control) == 0) {
    if (message == "ON") {
      digitalWrite(LED_PIN, HIGH); // Bật LED (giả định LED active HIGH)
      Serial.println("LED turned ON");
    } else if (message == "OFF") {
      digitalWrite(LED_PIN, LOW); // Tắt LED
      Serial.println("LED turned OFF");
    }
  }
}

void sendSensorData() {
  // 1. Đọc dữ liệu DHT11
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(5000);
    return;
  }
  
  Serial.print("Temperature: ");
  Serial.println(t);
  Serial.print("Humidity: ");
  Serial.println(h);
    
  
  // 2. Đọc dữ liệu LDR (Giá trị 0-4095)
  // int lightValue = analogRead(LDR_PIN);
    
  // Tạo chuỗi JSON để gửi lên Server
  String payload = "{";
  payload += "\"temperature\":" + String(t) + ",";
  payload += "\"humidity\":" + String(h) + ",";
  // payload += "\"light\":" + String(lightValue);
  payload += "}";

  // Gửi qua MQTT
  Serial.print("Publishing sensor data: ");
  Serial.println(payload);
  client.publish(mqtt_topic_data, payload.c_str());
  
}

// --- Setup ---
void setup() {
  Serial.begin(115200);
  Serial.println("--- Starting Application ---");
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Đảm bảo LED tắt khi khởi động

  setup_wifi();
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  dht.begin(); // Khởi động DHT
  delay(2000);  // ← ADD THIS: Give DHT11 time to stabilize

}

// --- Loop ---
void loop() {
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop(); // Xử lý các gói tin MQTT đến từ Server

  // Gửi dữ liệu định kỳ
  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;
    sendSensorData();
  }

  // float h = dht.readHumidity();
  // float t = dht.readTemperature();
  
  // Serial.print("Humidity: ");
  // Serial.print(h);
  // Serial.print("%  Temperature: ");
  // Serial.print(t);
  // Serial.println("°C");
  
  // delay(2000);
 }

// #include <DHT.h>

// #define DHTPIN 25
// #define DHTTYPE DHT11

// DHT dht(DHTPIN, DHTTYPE);

// void setup() {
//   Serial.begin(115200);
//   delay(1000);
//   Serial.println("\n\nDHT11 Test Starting...");
  
//   pinMode(DHTPIN, INPUT_PULLUP); // Enable internal pull-up
//   dht.begin();
//   delay(2000);
//   Serial.println("DHT11 initialized, waiting for readings...");
// }

// void loop() {
//   delay(3000); // DHT11 needs at least 2 seconds between reads
  
//   Serial.println("\n--- Reading DHT11 ---");
//   float h = dht.readHumidity();
//   float t = dht.readTemperature();
  
//   Serial.print("Raw Humidity: ");
//   Serial.println(h);
//   Serial.print("Raw Temperature: ");
//   Serial.println(t);
  
//   if (isnan(h) || isnan(t)) {
//     Serial.println("❌ FAILED - Check connections!");
//   } else {
//     Serial.print("✓ SUCCESS - Humidity: ");
//     Serial.print(h);
//     Serial.print("%  Temperature: ");
//     Serial.print(t);
//     Serial.println("°C");
//   }
// }