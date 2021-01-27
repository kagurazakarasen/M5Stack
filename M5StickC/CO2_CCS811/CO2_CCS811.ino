// M5STICK-Cで、CCS811のセンサーの値を取得する
// Original Programed by Kazuyuki Eguchi
// Modifi Version vi Rasen Kagurazaka
 
#include <M5StickC.h>
#include <Wire.h>
#include "SparkFunCCS811.h"
 
#define CCS811_ADDR 0x5B //Default I2C Address
 
CCS811 myCCS811(CCS811_ADDR);
 
void setup() {
  M5.begin();
  M5.Axp.ScreenBreath(9); // バックライトの明るさ(7-15)
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setCursor(0, 0);
   
  Wire.begin(0,26);  // SDA-G0,SCL-G26  <-逆でした！＞＜


  /*--- このチェックルーチンはエラーで動作せず
  CCS811Core::status returnCode = myCCS811.begin();
  if (returnCode != CCS811Core::SENSOR_SUCCESS) {
    Serial.println(".begin() returned with an error.");
    while (1);
  }
  */
  // このように書き換え
  if (myCCS811.begin() == false)
  {
    Serial.print("CCS811 error. Please check wiring. Freezing...");
    while (1)
      ;
  }


}
 
void loop() {
  if (myCCS811.dataAvailable()) {
    myCCS811.readAlgorithmResults();
    uint16_t co2 = myCCS811.getCO2();
    uint16_t tvoc = myCCS811.getTVOC();
 
    unsigned long sec = millis() / 1000;
 
    M5.Lcd.fillScreen(TFT_BLACK);
     
    if(sec >= 1200) {
      M5.Lcd.setTextColor(TFT_WHITE);
    } else {
      M5.Lcd.setTextColor(TFT_BLUE);  //起動後20分間は精度が出ないので青色で薄く表示
    }

    if(co2>999){
      M5.Lcd.setTextColor(TFT_YELLOW);  // CO2濃度が1000以上なら黄色く  
    }
    if(co2>1999){
      M5.Lcd.setTextColor(TFT_RED);   // 2000を超えたら赤くする
    }


 
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.print("Co2= ");
    M5.Lcd.print(co2);
    M5.Lcd.println(" ppm");
 
    M5.Lcd.print("TVOC= ");
    M5.Lcd.print(tvoc);
    M5.Lcd.println(" ppb");
     
    M5.Lcd.print("Time= ");
    M5.Lcd.print(sec);
    M5.Lcd.println(" sec");
 
    //Serial.print (ArduinoIDE シリアルプロッタ対応)
    Serial.println("CCS811 data:");
    Serial.print("CO2(ppm):");
    Serial.print(co2);
    Serial.print(",");
    Serial.print("TVOC(ppb):");
    Serial.print(tvoc);
//    Serial.print(",sec:");
//    Serial.print(sec);
    Serial.println("");
  }
 
  delay(10);
}
