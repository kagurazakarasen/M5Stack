/*
    M5Stick-C用、TVOC/eCO2(SGP30)Unit
*/

#include <M5StickC.h>
#include "Adafruit_SGP30.h"

Adafruit_SGP30 sgp;

void setup() {
    M5.begin();
    M5.Axp.ScreenBreath(9); // バックライトの明るさ(7-15)
    Wire.begin(32, 33, 100000); // ← M5StickCのI2CはSDA-G32、SLC-G33
    M5.Lcd.setRotation(1);
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setCursor(0, 0);
  
    //Serial.begin(115200);
    Serial.println("SGP30 test");
    if (! sgp.begin()){
      Serial.println("Sensor not found :(");
      while (1);
    }

  Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);

}

void loop() {

  unsigned long sec = millis() / 1000;

  
  if (! sgp.IAQmeasure()) {
    Serial.println("Measurement failed");
    return;
  }
  Serial.print("TVOC "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");
  Serial.print("eCO2 "); Serial.print(sgp.eCO2); Serial.println(" ppm");


  M5.Lcd.fillScreen(TFT_BLACK);

    M5.Lcd.setCursor(0, 0);
    M5.Lcd.print("eCO2:");
    M5.Lcd.print(sgp.eCO2);
    M5.Lcd.println(" ppm");
    M5.Lcd.println("");
 
    M5.Lcd.print("TVOC:");
    M5.Lcd.print(sgp.TVOC);
    M5.Lcd.println(" ppb");
     
/*
    M5.Lcd.print("Time= ");
    M5.Lcd.print(sec);
    M5.Lcd.println(" sec");
*/  


    delay(1000);
}
