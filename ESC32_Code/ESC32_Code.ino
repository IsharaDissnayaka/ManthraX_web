// Include Firebase and WiFi libraries
#include <WiFi.h>
#include <FirebaseESP32.h>

// WiFi credentials
#define WIFI_SSID "ishara"
#define WIFI_PASSWORD "BloodKnight"

// Firebase credentials
#define FIREBASE_DATABASE_URL "https://iotbp-caec7-default-rtdb.firebaseio.com/"
#define FIREBASE_API_KEY "AIzaSyBb3AhKfla-SwMU05WRqW4K_Ozz6iwboNM"

// Email and Password for Firebase Authentication
#define USER_EMAIL "esp32@test.com"
#define USER_PASSWORD "12345678"

// User UID (unique ID you provided)
#define USER_UID "NZmOLrVGVthBqjAmnMBKVwIw05w1"

// Create Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Pins
#define VOLTAGE_SENSOR_PIN 35  // D35
#define CURRENT_SENSOR_PIN 34  // D34

// Calibration values
float voltageCalibration = 311.0;  // Adjust based on your ZMPT101B
float currentCalibration = 30.0;   // Adjust based on your SCT-013

// Variables
float voltageValue = 0.0;
float currentValue = 0.0;
float power = 0.0;
float energyAccumulated = 0.0;
unsigned long lastTime = 0;

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected!");

  // Firebase Setup
  config.api_key = FIREBASE_API_KEY;
  config.database_url = FIREBASE_DATABASE_URL;
  
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.time_zone = 0;  // Adjust if necessary

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Firebase initialization done.");

  if (!Firebase.ready()) {
    Serial.println("Firebase not ready!");
  } else {
    Serial.println("Firebase is ready!");
  }

  lastTime = millis();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi not connected!");
    return;
  }

  if (!Firebase.ready()) {
    Serial.println("Firebase not ready!");
    return;
  }

  voltageValue = readVoltageRMS();
  currentValue = readCurrentRMS();
  power = voltageValue * currentValue;

  unsigned long now = millis();
  float timeInHours = (now - lastTime) / 3600000.0;
  energyAccumulated += power * timeInHours;
  lastTime = now;

  // Send data to Firebase under user UID
  bool success = true;

  if (Firebase.setFloat(fbdo, "/Users/" USER_UID "/Voltage", voltageValue)) {
    Serial.println("✅ Voltage sent successfully!");
  } else {
    Serial.print("❌ Failed to send Voltage: ");
    Serial.println(fbdo.errorReason());
    success = false;
  }

  if (Firebase.setFloat(fbdo, "/Users/" USER_UID "/Current", currentValue)) {
    Serial.println("✅ Current sent successfully!");
  } else {
    Serial.print("❌ Failed to send Current: ");
    Serial.println(fbdo.errorReason());
    success = false;
  }

  if (Firebase.setFloat(fbdo, "/Users/" USER_UID "/Power", power)) {
    Serial.println("✅ Power sent successfully!");
  } else {
    Serial.print("❌ Failed to send Power: ");
    Serial.println(fbdo.errorReason());
    success = false;
  }

  if (Firebase.setFloat(fbdo, "/Users/" USER_UID "/Energy", energyAccumulated)) {
    Serial.println("✅ Energy sent successfully!");
  } else {
    Serial.print("❌ Failed to send Energy: ");
    Serial.println(fbdo.errorReason());
    success = false;
  }

  // Display Sensor Data always after sending
  Serial.println("\n----- Latest Sensor Readings -----");
  Serial.print("Voltage (V): ");
  Serial.println(voltageValue, 2);
  Serial.print("Current (A): ");
  Serial.println(currentValue, 2);
  Serial.print("Power (W): ");
  Serial.println(power, 2);
  Serial.print("Energy (kWh): ");
  Serial.println(energyAccumulated, 4);
  Serial.println("-----------------------------------\n");

  delay(1000); // Update every 1 second
}

// Read Voltage RMS
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
  float mean = sum / (float)count;
  float rms = sqrt(mean) * voltageCalibration;
  return rms;
}

// Read Current RMS
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
  float mean = sum / (float)count;
  float rms = sqrt(mean) * currentCalibration;
  return rms;
}

