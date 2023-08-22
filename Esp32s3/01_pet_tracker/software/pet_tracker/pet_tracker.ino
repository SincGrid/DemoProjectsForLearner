#include <TinyGPS++.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

// Sender phone number with country code.
// Not GSM module phone number
const String PHONE = "+917976933120";

// GSM Module RX pin to ESP32-S3 (UART0 RXD)
// GSM Module TX pin to ESP32-S3 (UART0 TXD)
#define rxPin 3 // Change to the appropriate pin
#define txPin 1 // Change to the appropriate pin
HardwareSerial sim800(0); // Use UART0 for ESP32-S3

#define RXD2 8
#define TXD2 9
HardwareSerial neogps(1); // Use UART1 for ESP32-S3

TinyGPSPlus gps;

String smsStatus, senderNumber, receivedDate, msg;
boolean isReply = false;

// OLED display configuration
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_SDA 6      // OLED display SDA pin (Connect to GPIO21)
#define OLED_SCL 7      // OLED display SCL pin (Connect to GPIO22)
#define OLED_RESET -1    // OLED display reset pin (use -1 if not used)
#define SSD1306_ADDRESS 0x3C // I2C address of the OLED display

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32-S3 serial initialize");

  sim800.begin(9600, SERIAL_8N1, rxPin, txPin);
  Serial.println("SIM800L serial initialize");

  neogps.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("NeoGPS serial initialize");

  smsStatus = "";
  senderNumber = "";
  receivedDate = "";
  msg = "";

  sim800.println("AT+CMGF=1"); // SMS text mode
  delay(1000);
  sim800.println("AT+CMGD=1,4"); // Delete all saved SMS
  delay(1000);

  // Initialize the OLED display
  delay(200);
if (!display.begin(SSD1306_ADDRESS, OLED_RESET, OLED_SDA, OLED_SCL)) {
  Serial.println(F("SSD1306 allocation failed"));
  for (;;)
    ;
}

  // Clear the display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.display();
}

void loop() {
  while (sim800.available()) {
    parseData(sim800.readString());
  }

  while (Serial.available()) {
    sim800.println(Serial.readString());
  }
}

void parseData(String buff) {
  Serial.println(buff);

  unsigned int len, index;

  index = buff.indexOf("\r");
  buff.remove(0, index + 2);
  buff.trim();

  if (buff != "OK") {
    index = buff.indexOf(":");
    String cmd = buff.substring(0, index);
    cmd.trim();

    buff.remove(0, index + 2);

    if (cmd == "+CMTI") {
      index = buff.indexOf(",");
      String temp = buff.substring(index + 1, buff.length());
      temp = "AT+CMGR=" + temp + "\r";
      sim800.println(temp);
    } else if (cmd == "+CMGR") {
      extractSms(buff);
      Serial.println("Sender Number: " + senderNumber);
      Serial.println("PHONE: " + PHONE);
      if (senderNumber == PHONE) {
        if (msg == "get location") {
          sendLocation();
        } else if (msg == "get speed") {
          sendSpeed();
        }
      }
      sim800.println("AT+CMGD=1,4");
      delay(1000);
      smsStatus = "";
      senderNumber = "";
      receivedDate = "";
      msg = "";
    }
  } else {
    // The result of AT Command is "OK"
  }
}
}

void extractSms(String buff) {
  unsigned int index;
  Serial.println(buff);

  index = buff.indexOf(",");
  smsStatus = buff.substring(1, index - 1);
  buff.remove(0, index + 2);

  senderNumber = buff.substring(0, 13);
  buff.remove(0, 19);

  receivedDate = buff.substring(0, 20);
  buff.remove(0, buff.indexOf("\r"));
  buff.trim();

  index = buff.indexOf("\n\r");
  buff = buff.substring(0, index);
  buff.trim();
  msg = buff;
  buff = "";
  msg.toLowerCase();
}

void sendLocation() {
  boolean newData = false;
  for (unsigned long start = millis(); millis() - start < 2000;) {
    while (neogps.available()) {
      if (gps.encode(neogps.read())) {
        newData = true;
      }
    }
  }
  if (newData) {
    newData = false;
    delay(300);

    sim800.print("AT+CMGF=1\r");
    delay(1000);
    sim800.print("AT+CMGS=\"" + PHONE + "\"\r");
    delay(1000);
    sim800.print("http://maps.google.com/maps?q=loc:");
    sim800.print(gps.location.lat(), 6);
    sim800.print(",");
    sim800.print(gps.location.lng(), 6);
    delay(100);
    sim800.write(0x1A);
    delay(1000);

    // Print latitude and longitude on the OLED display
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Latitude: " + String(gps.location.lat(), 6));
    display.println("Longitude: " + String(gps.location.lng(), 6));
    display.display();
  }
}

void sendSpeed() {
  boolean newData = false;
  for (unsigned long start = millis(); millis() - start < 2000;) {
    while (neogps.available()) {
      if (gps.encode(neogps.read())) {
        newData = true;
      }
    }
  }
  if (newData) {
    newData = false;
    Serial.print("Speed km/h= ");
    Serial.println(gps.speed.kmph());
    delay(300);

    sim800.print("AT+CMGF=1\r");
    delay(1000);
    sim800.print("AT+CMGS=\"" + PHONE + "\"\r");
    delay(1000);
    sim800.print("Speed km/h: ");
    sim800.print(gps.speed.kmph());
    delay(100);
    sim800.write(0x1A);
    delay(1000);
    Serial.println("GPS Speed SMS Sent Successfully.");
  }
}