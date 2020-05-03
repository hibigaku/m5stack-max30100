#include <M5Stack.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include "xbm.h"  //my bitmap

#define REPORTING_PERIOD_MS     1000

PulseOximeter pox;
float Heart_rate = 0;
uint8_t Spo2 = 0;
uint32_t tsLastReport = 0;

// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
  printHRandSPO2(true);
}

void setup()
{
  Serial.begin(115200); // to PC via USB
  Serial2.begin(9600, SERIAL_8N1, 16, 17);   // to Sigfox module

  M5.begin();
  M5.Power.begin();
  M5.Lcd.clear(BLACK);
  M5.Lcd.setTextSize(4);
  
  // Initialize sensor
  if (!pox.begin()) {
    M5.Lcd.println("FAILED");
    for(;;);
   } else {
    M5.Lcd.println("SUCCESS");
  }

  //Register a callback for the beat detection
  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop()
{
  M5.update();  //update button state
  pox.update(); //パルスオキシメーターの更新

  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    Heart_rate = pox.getHeartRate();
    Spo2 = pox.getSpO2();
    printHRandSPO2(false);
    tsLastReport = millis();
  }

  if (M5.BtnA.wasReleased()) {
    sendSigfoxMessage();
  }
}

void printHRandSPO2(bool beat)
{
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(4);
  if (beat) {
    M5.Lcd.drawXBitmap(0, 5, hb2_bmp, 64, 32, TFT_RED);
  } else {
    M5.Lcd.drawXBitmap(0, 5, hb1_bmp, 64, 32, TFT_WHITE);
  }
  M5.Lcd.setCursor(0,70);
  M5.Lcd.print("HR:   "); M5.Lcd.println(Heart_rate);
  M5.Lcd.print("SPO2: "); M5.Lcd.println(Spo2);  
}

void sendSigfoxMessage() 
{
  String msg = "AT$SF=" + String(Heart_rate, HEX) + String(Spo2, HEX);
  Serial2.println(msg);
  M5.Lcd.setCursor(0,160);
  M5.Lcd.println("MSG Sent!");
  delay(10000);
}
