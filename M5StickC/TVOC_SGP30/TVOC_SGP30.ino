/*
    M5Stick-C用、TVOC/eCO2(SGP30)Unit
*/

#include <M5StickC.h>
#include "Adafruit_SGP30.h"
#include "Ambient.h"

#include "myConfig.h" // 自分の環境の設定を入れておく


WiFiClient client;
Ambient ambient;


Adafruit_SGP30 sgp;

long last_millis = 0;
unsigned long last_Anvi_millis = 0;

void setup() {
    M5.begin();
    M5.Axp.ScreenBreath(9); // バックライトの明るさ(7-15)
    Wire.begin(32, 33, 100000); // ← M5StickCのI2CはSDA-G32、SLC-G33
    M5.Lcd.setRotation(1);
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextSize(4);
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

#ifdef WIFI_ON
  WiFi.begin(ssid, password);  //  Wi-Fi APに接続
  while (WiFi.status() != WL_CONNECTED) {  //  Wi-Fi AP接続待ち
      delay(100);
  }

  Serial.print("WiFi connected\r\nIP address: ");
  Serial.println(WiFi.localIP());

#endif

#ifdef AMBIENT_ON
  ambient.begin(channelId, writeKey, &client);  //  チャネルIDとライトキーを指定してAmbientの初期化
#endif
  

}

int i = 15;

void loop() {

  unsigned long sec = millis() / 1000;

  while(i > 0) {    // 15秒カウントダウン
    if(millis()- last_millis > 1000) {
      last_millis = millis();
      i--;
      M5.Lcd.fillScreen(TFT_BLACK);
      M5.Lcd.setTextSize(5);
      //M5.Lcd.setTextColor(TFT_WHITE);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.print(i);      
     }
  }
  M5.Lcd.setTextSize(2);
 
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

#ifdef AMBIENT_ON
  // 60秒に一回
  if(millis()- last_Anvi_millis> 60000) {
    last_Anvi_millis = millis();

      // Ambientデータセット
      ambient.set(1, String(sgp.eCO2).c_str());
      ambient.set(2, String(sgp.TVOC).c_str());
            
      // Ambient送出
      ambient.send();

  }
  
#endif

    
}
