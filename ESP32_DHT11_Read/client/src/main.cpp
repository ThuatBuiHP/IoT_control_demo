#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <HTTPClient.h>

// WiFi credentials
const char* ssid = "OrsCorp";
const char* password = "Tamchiduc68";
const char* url = "http://192.168.1.231:3000/";

// The data pin on the DHT sensor is connected to GPIO pin 4 on the ESP32
#define DHTPIN 4
#define DHTTYPE DHT11

// Represents DHT11 sensor connected to GPIO pin 4
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // put your setup code here, to run once:
  // Set up serial communication between your computer and ESP32 at 9600 baud
  Serial.begin(9600);
  // Connect ESP32 to your WiFi network
  WiFi.begin(ssid, password);
  // Initialize DHT sensor, prepare it for readings
  dht.begin();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("..");
    delay(500);
  }
  Serial.println("Connected ESP32 to WiFi");
}

void loop() {
  // Read temperature and humidity from DHT sensor
  // If readings are valid, save them to variables

  // Read temperature and humidity from DHT sensor
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  
  // Check if readings are valid
  if (isnan(temp) || isnan(hum)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(5000);
    return;
  }
  
  Serial.print("Temperature: ");
  Serial.println(temp);
  Serial.print("Humidity: ");
  Serial.println(hum);

  // Send data to server by calling POST API
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(5000);  // Thêm dòng này - timeout 5 giây
    
    String jsonData = "{\"temperature\":" + String(temp) + ",\"humidity\":" + String(hum) + "}";
    Serial.print("Sending JSON: ");
    Serial.println(jsonData);  // Kiểm tra JSON format
    
    int res = http.POST(jsonData);
    Serial.print("HTTP Response code: ");
    Serial.println(res);
    
    if (res > 0) {
      Serial.println("Response from server:");
      Serial.println(http.getString());
    } else {
      Serial.print("Error code: ");
      Serial.println(res);
    }
    
    http.end();
  }
  delay(5000);
}