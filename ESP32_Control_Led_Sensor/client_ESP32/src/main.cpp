#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// --- Cấu hình Mạng & MQTT ---
const char* ssid = "TEN_WIFI_CUA_BAN";
const char* password = "MAT_KHAU_CUA_BAN";
const char* mqtt_server = "broker.hivemq.com"; 
const char* mqtt_client_id = "ESP32_Smart_Client";

// Chủ đề (Topic)
const char* mqtt_topic_control = "home/led/control"; // Nhận lệnh
const char* mqtt_topic_data = "home/sensor/data";   // Gửi dữ liệu

// --- Cấu hình Cảm biến & LED ---
const int LED_PIN = 2;        // Chân LED (ví dụ: GPIO 2)
const int LDR_PIN = 34;       // Chân Analog cho LDR (Chỉ dùng các chân có ADC - ví dụ: GPIO 32-39)
const int DHT_PIN = 23;       // Chân Digital cho DHT11
#define DHTTYPE DHT11         // Loại cảm biến

// Khai báo đối tượng
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHT_PIN, DHTTYPE);

unsigned long lastMsg = 0;
const long interval = 5000; // Gửi dữ liệu mỗi 5 giây

// --- Hàm khai báo ---
void setup_wifi();
void reconnect_mqtt();
void callback(char* topic, byte* payload, unsigned int length);
void sendSensorData();

// --- Các hàm đã cập nhật ---
void setup_wifi() { /* Giữ nguyên như code cũ */
  delay(10);
  Serial.begin(115200);
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

void reconnect_mqtt() { /* Giữ nguyên như code cũ */
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqtt_client_id)) {
      Serial.println("connected");
      client.subscribe(mqtt_topic_control);
      Serial.print("Subscribed to topic: ");
      Serial.println(mqtt_topic_control);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) { /* Giữ nguyên logic điều khiển LED */
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  if (strcmp(topic, mqtt_topic_control) == 0) {
    if (message == "ON") {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("LED turned ON");
    } else if (message == "OFF") {
      digitalWrite(LED_PIN, LOW);
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
    // Không gửi dữ liệu nếu đọc thất bại
  } else {
    // 2. Đọc dữ liệu LDR (Giá trị 0-4095)
    int lightValue = analogRead(LDR_PIN);
    
    // Tạo chuỗi JSON để gửi lên Server
    String payload = "{";
    payload += "\"temperature\":" + String(t) + ",";
    payload += "\"humidity\":" + String(h) + ",";
    payload += "\"light\":" + String(lightValue);
    payload += "}";

    // Gửi qua MQTT
    Serial.print("Publishing sensor data: ");
    Serial.println(payload);
    client.publish(mqtt_topic_data, payload.c_str());
  }
}

// --- Setup ---
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  setup_wifi();
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  dht.begin(); // Khởi động DHT
}

// --- Loop ---
void loop() {
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();

  // Gửi dữ liệu định kỳ
  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;
    sendSensorData();
  }
}