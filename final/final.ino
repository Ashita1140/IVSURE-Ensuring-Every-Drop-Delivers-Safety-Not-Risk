#define BLYNK_TEMPLATE_ID "TMPL3trhwMivJ"
#define BLYNK_TEMPLATE_NAME "IV sure"
#define BLYNK_AUTH_TOKEN "aapHDMH9-rYCYnSCyZeHwjEjFwcqKNCh"

#define BLYNK_PRINT Serial

#include "HX711.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// HX711 pins
#define DOUT 23
#define SCK  19

HX711 scale;

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// WiFi credentials
const char* ssid = "Redmi 13C 5G";
const char* password = "bansal23";

// Calibration
float calibration_factor = -38;
bool alertSent = false;

void setup() {
  Serial.begin(115200);

  // HX711
  scale.begin(DOUT, SCK);
  scale.set_scale(calibration_factor);
  scale.tare();

  // LCD
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();

  lcd.print("Connecting WiFi");

  // WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  lcd.clear();
  lcd.print("WiFi Connected");
  delay(2000);

  // Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  lcd.clear();
}


void loop() {
  Blynk.run();

  float weight = scale.get_units(10);

  String status;
  float drip = weight * 0.05;

  // Status logic
  if (weight <= 50) {
    status = "EMPTY";
  } 
  else if (weight <= 200) {
    status = "LOW";
  } 
  else if (weight <= 400) {
    status = "MEDIUM";
  } 
  else {
    status = "FULL";
  }


   /** STATUS + COLOR LOGIC **/

  // EMPTY → RED
  if (weight <= 50) {

    status = "EMPTY";

    Blynk.setProperty(V2, "color", "#FF0000"); // RED
    Blynk.virtualWrite(V2, 1);

    // Notification only once
    if (!alertSent) {

      Blynk.logEvent(
        "iv_empty",
        "🚨 WARNING! IV Bottle is Almost Empty ⚠️💉\nPlease Replace Immediately 🏥"
      );

      Serial.println("🚨 Notification Sent!");

      alertSent = true;
    }
  }

  // LOW → ORANGE
  else if (weight <= 200) {

    status = "LOW";

    Blynk.setProperty(V2, "color", "#FFA500"); // ORANGE
    Blynk.virtualWrite(V2, 1);

    alertSent = false;
  }

  // MEDIUM → YELLOW
  else if (weight <= 400) {

    status = "MEDIUM";

    Blynk.setProperty(V2, "color", "#FFFF00"); // YELLOW
    Blynk.virtualWrite(V2, 1);

    alertSent = false;
  }

  // FULL → GREEN
  else {

    status = "FULL";

    Blynk.setProperty(V2, "color", "#00FF00"); // GREEN
    Blynk.virtualWrite(V2, 1);

    alertSent = false;
  }


  // LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IV:");
  lcd.print(status);

  lcd.setCursor(0, 1);
  lcd.print("Wt:");
  lcd.print(weight);

  // Serial
  Serial.print("Weight: ");
  Serial.print(weight);
  Serial.print(" g | Status: ");
  Serial.println(status);

  // Blynk
  Blynk.virtualWrite(V1, weight);   // bottle level
  Blynk.virtualWrite(V3, status);   // status
  Blynk.virtualWrite(V0, drip);     // drip rate

  delay(1000);
}