#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPL3trhwMivJ"
#define BLYNK_TEMPLATE_NAME "IV sure"
#define BLYNK_AUTH_TOKEN "aapHDMH9-rYCYnSCyZeHwjEjFwcqKNCh"

#include <HX711.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// WIFI
char ssid[] = "Redmi 13C 5G";
char pass[] = "bansal23";

// HX711
#define DOUT 23
#define CLK  19

HX711 scale;

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// CALIBRATION
float calibration_factor = -38;
bool alertSent = false;

void setup() {
  Serial.begin(115200);

  // HX711
  scale.begin(DOUT, CLK);
  scale.set_scale(calibration_factor);
  scale.tare();

  // LCD
  lcd.init();
  lcd.backlight();

  // Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  lcd.setCursor(0, 0);
  lcd.print("IV System Ready");
  delay(2000);
  lcd.clear();
}


void loop() {
  Blynk.run();

  // ✔ Correct weight reading
  float weight = scale.get_units(10);

  String status;
  float drip = weight * 0.05;

  // IV LEVEL LOGIC
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


  // SERIAL
  Serial.print("Weight: ");
  Serial.print(weight);
  Serial.print(" g | Status: ");
  Serial.println(status);

  // LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IV Status:");
  lcd.setCursor(0, 1);
  lcd.print(status);

  // BLYNK OUTPUT
  Blynk.virtualWrite(V1, weight);   // Bottle level
  Blynk.virtualWrite(V3, status);   // Status
  Blynk.virtualWrite(V0, drip);     // Drip rate

  delay(1000);
}