#include <Wire.h>
#include <WiFi.h>

const char *ssid = "MʀCoTcH";
const char *password = "MrCoTcH@GAMER";

// I2C address of the sunlight sensor
const int sunlightSensorAddress = 0x60;

// Pin definitions
const int moistureSensorPin = 1;   // Connect non-corrosive soil moisture sensor to GPIO33_ADC
const int ldrDigitalPin = 8;       // Digital pin for LDR module
const int relayControlPin = 12;    // Digital pin for relay control (choose any available GPIO pin)

const int moistureThreshold = 50; // Adjust this threshold as needed

void setup() {
  pinMode(ldrDigitalPin, INPUT); // Set LDR pin as input
  pinMode(relayControlPin, OUTPUT); // Set relay control pin as output

  digitalWrite(relayControlPin, LOW); // Turn off the relay initially

  Wire.begin();
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  int moistureValue = analogRead(moistureSensorPin);
  int ldrValue = digitalRead(ldrDigitalPin); // Read digital LDR pin

  // Read sunlight percentage from I2C sunlight sensor
  Wire.beginTransmission(sunlightSensorAddress);
  Wire.write(0x02); // Register address for sunlight percentage
  Wire.endTransmission(false);
  Wire.requestFrom(sunlightSensorAddress, 2);

  int sunlightValue = Wire.read() << 8 | Wire.read();
  float sunlightPercentage = (sunlightValue / 65535.0) * 100.0;

  // Convert analog readings to meaningful values
  int moisturePercentage = map(moistureValue, 0, 4095, 0, 100);

  // Determine LDR status based on digital value
  String ldrStatus = (ldrValue == HIGH) ? "Dark" : "Bright";

  // Control the relay (and pump) based on moisture level
  if (moisturePercentage > moistureThreshold) {
    digitalWrite(relayControlPin, HIGH); // Turn on the relay (and pump)
  } else {
    digitalWrite(relayControlPin, LOW); // Turn off the relay (and pump)
  }

  // Print the sensor data to the Serial Monitor
  Serial.println("Sensor Data:");
  Serial.print("Sunlight Percentage: ");
  Serial.print(sunlightPercentage);
  Serial.println("%");
  Serial.print("Moisture Percentage: ");
  Serial.print(moisturePercentage);
  Serial.println("%");
  Serial.print("LDR Status: ");
  Serial.println(ldrStatus);

  delay(5000); // Wait for a few seconds before the next reading
}