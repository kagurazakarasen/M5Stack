// M5Atomで、CCS811のセンサーの値を取得する
// Original M5StickC Ver Programed by Kazuyuki Eguchi
// Modifi Version by Rasen Kagurazaka
 
//#include <M5StickC.h>
#include "M5Atom.h"
#include <Wire.h>
#include "SparkFunCCS811.h"
 
#define CCS811_ADDR 0x5B //Default I2C Address

#define WHITE 0x707070
#define GREEN 0xf00000
#define RED   0x00f000
#define BLUE  0x0000f0
#define YELLOW  0xf0f000
#define BLACK 0x0

CCS811 myCCS811(CCS811_ADDR);
 
void setup() {
  M5.begin(true, false, true);
  //M5.begin();
   
  Wire.begin(22,19);  // SDA-G22,SCL-G19

  if (myCCS811.begin() == false)
  {
    Serial.print("CCS811 error. Please check wiring. Freezing...");
    while (1)
      ;
  }
  Serial.println("CCS811 START");
}
 
void loop() {
  if (myCCS811.dataAvailable()) {

    
    myCCS811.readAlgorithmResults();
    uint16_t co2 = myCCS811.getCO2();
    uint16_t tvoc = myCCS811.getTVOC();

    uint16_t c = 400;

    unsigned long sec = millis() / 1000;

//    unsigned long colr = WHITE;

    if(sec >= 1200) {
      for(int i=0;i<26;i++) M5.dis.drawpix(i, BLACK);
    } else {
      //起動後20分間は精度が出ないので背景を薄い色で
      for(int i=0;i<26;i++) M5.dis.drawpix(i, 0x101010); //薄い色で塗りつぶし 点滅に見える……
      Serial.println("ageing now..");
    }

    // 400ppmから＋500刻みで、青、緑、白、黄、赤と変化するバーグラフかなり適当版
    if(co2>400 && co2<900){
      c = co2 - 400;
      c = int(c/2);
      c = int(c/10); 
      for(int i=0;i<c;i++) M5.dis.drawpix(i, BLUE);
    }

    if(co2>900 && co2<1400){
      c = co2 - 900;
      c = int(c/2);
      c = int(c/10); 
      for(int i=0;i<c;i++) M5.dis.drawpix(i, GREEN);
    }

    if(co2>1400 && co2<1900){
      c = co2 - 1400;
      c = int(c/2);
      c = int(c/10); 
      for(int i=0;i<c;i++) M5.dis.drawpix(i, WHITE);
    }

    if(co2>1900 && co2<2400){
      c = co2 - 1900;
      c = int(c/2);
      c = int(c/10); 
      for(int i=0;i<c;i++) M5.dis.drawpix(i, YELLOW);
    }

    if(co2>2400){
      c = co2 - 900;
      c = int(c/2);
      c = int(c/10); 
      for(int i=0;i<c;i++) M5.dis.drawpix(i, RED);
    }

 
    //Serial.print (ArduinoIDE シリアルプロッタ対応)
    Serial.println("CCS811 data:");
    Serial.print("CO2(ppm):");
    Serial.print(co2);
    Serial.print(",");
    Serial.print("TVOC(ppb):");
    Serial.print(tvoc);
    Serial.print(",sec:");
    Serial.print(sec);
    Serial.println("");
  }
 
  delay(10);
}
