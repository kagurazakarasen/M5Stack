#include "M5Atom.h"
#include <Wire.h>
#include "Adafruit_Sensor.h"
#include <Adafruit_BMP280.h>
#include "SHT3X.h"
SHT3X sht30;
Adafruit_BMP280 bme;

float tmp = 0.0;
float hum = 0.0;
float pressure = 0.0;

#define WHITE 0x707070
#define GREEN 0xf00000
#define RED   0x00f000
#define BLUE  0x0000f0
#define YELLOW  0xf0f000
#define BLACK 0x0


void setup()
{
    M5.begin(true, false, true);
//    M5.Power.begin();
    Wire.begin();
    delay(50);

    Serial.println(F("ENV Unit(SHT30 and BMP280) test..."));
    while (!bme.begin(0x76)){  
      Serial.println("Could not find a valid BMP280 sensor, check wiring!");
      M5.dis.drawpix(24, 0x00f000);  //Red
    }
  
    M5.dis.drawpix(0, 0xf00000);
}

void TempDisp(){  // rangeは10の位の数。０＝0～９、1＝１０～１９・・・

  //tmpはグローバル、そこから計算できる

  int range;
  long cc; // color


  if(tmp>-20){
    range=-2;
    cc = WHITE;
  }

  if(tmp>-10){
    range=-1;
    cc = WHITE;
  }

  if(tmp>0){
    range=0;
    cc = WHITE;
  }
  if(tmp>10){
    range=1;
    cc = BLUE;
  }
  if(tmp>20){
    range=2;
    cc = GREEN;
  }
  if(tmp>30){
    range=3;
    cc = YELLOW;
  }
  if(tmp>40){
    range=4;
    cc = RED;
  }
  
  float tt = tmp - range*10;
  float tmp2 = (int)(tt*10) / 10.0;  //小数点２位以下切り捨て
  int tmp3 = (int)tmp2;  // 整数値
  int tmp4 = (int)((tmp2 - (float)tmp3)*10.0);  //小数点以下（１桁）

  Serial.printf("tt: %2.2f*C \r\n", tt);
  Serial.printf("Temp2: %2.2f \r\n", tmp2);
  Serial.printf("Temp3: %d \r\n", tmp3);
  Serial.printf("Temp4: %d \r\n", tmp4);  


  int i;

    // tt0～10
    for(i=0;i<11;i++){
      if(i<tmp3) M5.dis.drawpix(i, cc);
      else M5.dis.drawpix(i, BLACK);
    }

    //10の位
    for(i=10;i<15;i++)  M5.dis.drawpix(i, BLACK);
    if(range>=0){
      M5.dis.drawpix(9+range, WHITE); 
    }else{
      M5.dis.drawpix(14+range, BLUE); 
    }

    
    for(i=0;i<tmp4;i++){
      M5.dis.drawpix(i+15, cc);
    }
    for(i=tmp4;i<11;i++) M5.dis.drawpix(i+15, BLACK);



}



void loop()
{

  long colr;

  pressure = bme.readPressure();
  if(sht30.get()==0){
    tmp = sht30.cTemp;
    hum = sht30.humidity;
  }
  Serial.printf("Temperatura: %2.2f*C  Humedad: %0.2f%%  Pressure: %0.2fPa\r\n", tmp, hum, pressure);


  float tmp2 = (int)(tmp*10) / 10.0;  //小数点２位以下切り捨て

  int tmp3 = (int)tmp2;  // 整数値

  int tmp4 = (int)((tmp2 - (float)tmp3)*10.0);  //小数点以下（１桁）

/*
  Serial.printf("Temp2: %2.2f*C \r\n", tmp2);
  Serial.printf("Temp3: %d*C \r\n", tmp3);
  Serial.printf("Temp4: %d \r\n", tmp4);  
*/

  int i;

  //for(i=10;i<15;i++)  M5.dis.drawpix(i, BLACK);
  //M5.dis.drawpix(11, 0x707070);  //White 小数点

/*
  if(tmp3<0){ // 氷点下時

    if(tmp3<-10){ // マイナス10度以下
       for(i=0;i<25+1;i++){
        if(i<(tmp3*(-1))){
          M5.dis.drawpix(i, 0x707070);  //White
        }else{
          M5.dis.drawpix(i, 0x0);  //BLK
        }         
       }
    }else{
      // -10.0 ～0.0
      for(i=0;i<(tmp3*-1);i++){
        M5.dis.drawpix(i, 0x707070);  //White
      }
      M5.dis.drawpix(10, BLUE);  //White 小数点
      for(i=0;i<tmp4;i++){
        M5.dis.drawpix(i+15, 0x707070);  //White
      }
      for(i=tmp4;i<11;i++) M5.dis.drawpix(i+15, BLACK);
    }
    
  } else { // プラス気温
*/

    TempDisp();

/*
    if(tmp3>0 and tmp3<11){

      TempDisp(tmp,WHITE);
    }

    if(tmp3>10 and tmp3<21){
        TempDisp(tmp-10.0,BLUE);
    }

    if(tmp3>20 and tmp3<31){
        TempDisp(tmp-20.0,GREEN);
    }

    if(tmp3>30 and tmp3<41){
        TempDisp(tmp-30.0,RED);
    }
    if(tmp3>40 and tmp3<51){
        TempDisp(tmp-40.0,RED);
    }
*/


/*    
    if(tmp3>0 and tmp3<11){
      // 0度以上10度未満
      for(i=0;i<11;i++){
        if(i<tmp3) M5.dis.drawpix(i, WHITE);
        else M5.dis.drawpix(i, BLACK);
      }
      M5.dis.drawpix(9, WHITE);  //小数点
      for(i=0;i<tmp4;i++){
        M5.dis.drawpix(i+15, WHITE);
      }
      for(i=tmp4;i<11;i++) M5.dis.drawpix(i+15, BLACK);
      //if(tmp3==0) M5.dis.drawpix(9, WHITE);
    }


    if(tmp3>10 and tmp3<21){
      // 10度以上20度未満
      for(i=0;i<11;i++){
        if(i<tmp3-10) M5.dis.drawpix(i, BLUE);
        else M5.dis.drawpix(i, BLACK);
      }
      M5.dis.drawpix(10, WHITE);  //小数点
      for(i=0;i<tmp4;i++){
        M5.dis.drawpix(i+15, BLUE);
      }
      for(i=tmp4;i<11;i++) M5.dis.drawpix(i+15, BLACK);
      //if(tmp3==10) M5.dis.drawpix(9, BLUE);
    }

    
    if(tmp3>20 and tmp3<31){
      // 20度以上30度未満
      for(i=0;i<11;i++){
        if(i<tmp3-20) M5.dis.drawpix(i, GREEN);
        else M5.dis.drawpix(i, BLACK);
      }
      M5.dis.drawpix(11, WHITE);  //小数点
      for(i=0;i<tmp4;i++){
        M5.dis.drawpix(i+15, GREEN);
      }
      for(i=tmp4;i<11;i++) M5.dis.drawpix(i+15, BLACK);
      //if(tmp3==20) M5.dis.drawpix(9, GREEN);
    }

    
    if(tmp3>30 and tmp3<41){
      // 30度以上40度未満
      for(i=0;i<11;i++){
        if(i<tmp3-30) M5.dis.drawpix(i, RED);
        else M5.dis.drawpix(i, BLACK);
      }
      M5.dis.drawpix(12, WHITE);  //小数点
      for(i=0;i<tmp4;i++){
        M5.dis.drawpix(i+15, RED);
      }
      for(i=tmp4;i<11;i++) M5.dis.drawpix(i+15, BLACK);
      //if(tmp3==30) M5.dis.drawpix(9, RED);
    }

    if(tmp3>40 and tmp3<51){
      // 40度以上50度未満
      for(i=0;i<11;i++){
        if(i<tmp3-40) M5.dis.drawpix(i, RED);
        else M5.dis.drawpix(i, BLACK);
      }
      M5.dis.drawpix(13, WHITE);  //小数点
      for(i=0;i<tmp4;i++){
        M5.dis.drawpix(i+15, RED);
      }
      for(i=tmp4;i<11;i++) M5.dis.drawpix(i+15, BLACK);
      //if(tmp3==30) M5.dis.drawpix(9, RED);
    }

*/
    
//  }

    if (M5.Btn.wasPressed())
    {
      // ボタンが押された時の動作
    }

    delay(100);
    M5.update();
}
