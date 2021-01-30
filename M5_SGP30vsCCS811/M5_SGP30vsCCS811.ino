/*
  M5Stackのサンプルスケッチの TVOC_SGP30を改造
  TVOC/eCO2 UNIT と　CCS811 の両方からCO2を測定、それぞれのデータをAmbientへ送る
*/

#include <M5Stack.h>
#include "Adafruit_SGP30.h"
#include <SparkFunCCS811.h> //Click here to get the library: http://librarymanager/All#SparkFun_CCS811
#include "Ambient.h"

#include "myConfig.h" // 自分の環境の設定を入れておく

WiFiClient client;
Ambient ambient;

#define CCS811_ADDR 0x5B //Default I2C Address

//Global sensor objects
CCS811 myCCS811(CCS811_ADDR);

Adafruit_SGP30 sgp;
int i = 15;
long last_millis = 0;
void header(const char *string, uint16_t color)
{
    M5.Lcd.fillScreen(color);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.fillRect(0, 0, 320, 30, TFT_BLACK);
    M5.Lcd.setTextDatum(TC_DATUM);
    M5.Lcd.drawString(string, 160, 3, 4); 
}

void setup() {
  M5.begin(true, false, true, true);
  header("CO2&TVOC",TFT_BLACK);
  //Serial.begin(115200);   // ＜＝こいつ要らないんじゃ？

  WiFi.begin(ssid, password);  //  Wi-Fi APに接続
  while (WiFi.status() != WL_CONNECTED) {  //  Wi-Fi AP接続待ち
      delay(100);
  }

  Serial.print("WiFi connected\r\nIP address: ");
  Serial.println(WiFi.localIP());

  ambient.begin(channelId, writeKey, &client);  //  チャネルIDとライトキーを指定してAmbientの初期化
         



  Serial.println("SGP30 test");
  if (! sgp.begin()){
    Serial.println("SGP30 Sensor not found :(");
    while (1);
  }

  //This begins the CCS811 sensor and prints error status of .beginWithStatus()
  CCS811Core::CCS811_Status_e returnCode = myCCS811.beginWithStatus();
  Serial.print("CCS811 begin exited with: ");
  Serial.println(myCCS811.statusString(returnCode));
  

  M5.Lcd.drawString("SGP30:", 10, 20, 4);
  M5.Lcd.drawString("TVOC:", 50, 40, 4);
  M5.Lcd.drawString("eCO2:", 50, 60, 4);

  M5.Lcd.drawString("CCS811:", 10, 100, 4);
  M5.Lcd.drawString("TVOC:", 50, 120, 4);
  M5.Lcd.drawString("eCO2:", 50, 140, 4);

  Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);
  M5.Lcd.drawString("Initialization...", 140, 180, 4);
}

void loop() {
  while(i > 0) {    
    if(millis()- last_millis > 1000) {
      last_millis = millis();
      i--;
      M5.Lcd.fillRect(198, 180, 40, 20, TFT_BLUE);
      M5.Lcd.drawNumber(i, 20, 180, 4);
    }
  }
  M5.Lcd.fillRect(0, 180, 300, 30, TFT_BLACK);

  if (! myCCS811.dataAvailable()){
     Serial.println("CCS811 Not Available");
    return;
  }

  if (! sgp.IAQmeasure()) {
    Serial.println("Measurement failed");
    return;
  }
  M5.Lcd.fillRect(90, 40, 80, 190, TFT_BLACK);
  M5.Lcd.drawNumber(sgp.TVOC, 120, 40 , 4);
  M5.Lcd.drawString("ppb", 200, 40, 4);
  M5.Lcd.drawNumber(sgp.eCO2, 120, 60, 4);
  M5.Lcd.drawString("ppm", 200, 60, 4);

  Serial.print("SGP30:TVOC(ppb) "); Serial.print(sgp.TVOC); Serial.print(",");
  Serial.print("SGP30:eCO2(ppm) "); Serial.print(sgp.eCO2); Serial.print(",");

  //CCS811
  myCCS811.readAlgorithmResults();

  uint16_t co2 = myCCS811.getCO2();
  uint16_t tvoc = myCCS811.getTVOC();

  M5.Lcd.drawNumber(tvoc, 120, 120 , 4);
  M5.Lcd.drawString("ppb", 200, 120, 4);
  M5.Lcd.drawNumber(co2, 120, 140, 4);
  M5.Lcd.drawString("ppm", 200, 140, 4);


  Serial.print("CCS811:TVOC(ppb) "); Serial.print(tvoc); Serial.print(",");
  Serial.print("CCS811:eCO2(ppm) "); Serial.print(co2); Serial.println("");

  // Ambientデータセット
  ambient.set(1, String(co2).c_str());
  ambient.set(2, String(tvoc).c_str());

  ambient.set(3, String(sgp.eCO2).c_str());
  ambient.set(4, String(sgp.TVOC).c_str());

  // Ambient送出
  ambient.send();
 
  delay(5000);
}
