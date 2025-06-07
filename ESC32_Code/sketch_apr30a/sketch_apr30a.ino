#include <WiFi.h>
#include <FirebaseESP32.h>
#include <math.h>

// WiFi credentials
#define WIFI_SSID "Dialog 4G"
#define WIFI_PASSWORD "T4AML535ET3"

// Firebase credentials
#define FIREBASE_DATABASE_URL "https://iotbp-caec7-default-rtdb.firebaseio.com/"
#define FIREBASE_API_KEY "AIzaSyBb3AhKfla-SwMU05WRqW4K_Ozz6iwboNM"
#define USER_EMAIL "esp32@test.com"
#define USER_PASSWORD "12345678"
#define USER_UID "NZmOLrVGVthBqjAmnMBKVwIw05w1"

// Firebase and WiFi objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Pins and calibration
#define VOLTAGE_SENSOR_PIN 35
#define CURRENT_SENSOR_PIN 34
float voltageCalibration = 311.0;
float currentCalibration = 30.0;

// Variables
float voltageValue = 0.0, currentValue = 0.0, power = 0.0, energyAccumulated = 0.0;
unsigned long lastTime = 0;

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Wi-Fi Connected!");

  config.api_key = FIREBASE_API_KEY;
  config.database_url = FIREBASE_DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  lastTime = millis();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED || !Firebase.ready()) return;

  voltageValue = readVoltageRMS();
  currentValue = readCurrentRMS();
  power = voltageValue * currentValue;

  unsigned long now = millis();
  float timeInHours = (now - lastTime) / 3600000.0;
  energyAccumulated += power * timeInHours;
  lastTime = now;

  // Create JSON object for batch write
  FirebaseJson json;
  json.set("Voltage", voltageValue);
  json.set("Current", currentValue);
  json.set("Power", power);
  json.set("Energy", energyAccumulated);

  String path = "/Users/" + String(USER_UID);

  if (Firebase.updateNode(fbdo, path, json)) {
    Serial.println("✅ Firebase data sent successfully!");
  } else {
    Serial.print("❌ Firebase Error: ");
    Serial.println(fbdo.errorReason());
  }

  // Log to serial
  Serial.println("----- Latest Sensor Readings -----");
  Serial.printf("Voltage (V): %.2f\n", voltageValue);
  Serial.printf("Current (A): %.2f\n", currentValue);
  Serial.printf("Power (W): %.2f\n", power);
  Serial.printf("Energy (kWh): %.4f\n", energyAccumulated);
  Serial.println("-----------------------------------\n");

  delay(1000);
}

float readVoltageRMS() {
  long sum = 0;
  int count = 0;
  unsigned long start_time = millis();
  while (millis() - start_time < 100) {
    int adc = analogRead(VOLTAGE_SENSOR_PIN);
    float voltage = ((adc / 4095.0) * 3.3) - 1.65;
    sum += voltage * voltage;
    count++;
  }
  return sqrt(sum / (float)count) * voltageCalibration;
}

float readCurrentRMS() {
  long sum = 0;
  int count = 0;
  unsigned long start_time = millis();
  while (millis() - start_time < 100) {
    int adc = analogRead(CURRENT_SENSOR_PIN);
    float voltage = ((adc / 4095.0) * 3.3) - 1.65;
    sum += voltage * voltage;
    count++;
  }
  return sqrt(sum / (float)count) * currentCalibration;
}
