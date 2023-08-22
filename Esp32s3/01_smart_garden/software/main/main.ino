#include <Wire.h>
#include <WiFi.h>
#include <DHT.h>

const char *ssid = "error";
const char *password = "mayank13";



// Pin definitions
const int moistureSensorPin = 16;   // Connect non-corrosive soil moisture sensor to GPIO33_ADC
const int ldrDigitalPin = 5;        // Digital pin for LDR module
const int relayControlPin = 6; 
const int relayControlPin = 7;      // Digital pin for relay control (choose any available GPIO pin)
const int dhtPin = 4;               // Digital pin for DHT11 sensor

const int moistureThreshold = 50; // Adjust this threshold as needed

DHT dht(dhtPin, DHT11); // Initialize DHT11 sensor

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

  dht.begin(); // Initialize DHT sensor
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

  // Read temperature and humidity from DHT11 sensor
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

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
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println("°C");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");

  delay(5000); // Wait for a few seconds before the next reading
}