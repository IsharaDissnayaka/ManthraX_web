// Only WiFi library is removed since not needed
// Firebase libraries are removed

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
  lastTime = millis();
}

void loop() {
  voltageValue = readVoltageRMS();
  currentValue = readCurrentRMS();
  power = voltageValue * currentValue;

  unsigned long now = millis();
  float timeInHours = (now - lastTime) / 3600000.0;
  energyAccumulated += power * timeInHours;
  lastTime = now;

  // Display Sensor Data
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
