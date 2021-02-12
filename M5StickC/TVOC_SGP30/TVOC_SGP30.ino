/*
    note: need add library Adafruit_BMP280 from library manage
*/

#include <M5StickC.h>
#include "Adafruit_SGP30.h"

Adafruit_SGP30 sgp;

void setup() {
    M5.begin();
    Wire.begin(32, 33, 100000);
    M5.Lcd.setRotation(3);

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
 
  if (! sgp.IAQmeasure()) {
    Serial.println("Measurement failed");
    return;
  }
  Serial.print("TVOC "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");
  Serial.print("eCO2 "); Serial.print(sgp.eCO2); Serial.println(" ppm");


    delay(1000);
}
