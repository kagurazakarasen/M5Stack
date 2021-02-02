/*
    SGP30(eCO2)センサーとENV-II両方測定。Ambientにも出力    
*/


#include <M5Stack.h>
#include "Adafruit_SGP30.h"


//Ambient
#include "Ambient.h"

#include "myConfig.h" // 自分の環境の設定を入れておく

WiFiClient client;
Ambient ambient;


//ENV_II
#include "Adafruit_Sensor.h"
#include <Adafruit_BMP280.h>
#include "SHT3X.h"
SHT3X sht30;
Adafruit_BMP280 bme;

float tmp = 0.0;
float hum = 0.0;
float pressure = 0.0;
//




Adafruit_SGP30 sgp;
int i = 15;
long last_millis = 0;

unsigned long last_Anvi_millis = 0;

void header(const char *string, uint16_t color)
{
    M5.Lcd.fillScreen(color);
    //M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.fillRect(0, 0, 320, 30, TFT_BLACK);
    //M5.Lcd.setTextDatum(TC_DATUM); // TopCenter！？
    //M5.Lcd.drawString(string, 160, 3, 4); 
    M5.Lcd.drawCentreString(string, 160, 3, 4);
}



void setup() {
  M5.begin(true, false, true, true);
  header("SGP30 and ENV2 ",TFT_BLACK);
  //Serial.begin(115200);
  Serial.println("SGP30 and ENV2 test");
  
  if (! sgp.begin()){
    Serial.println("SGP30 not found :(");
    while (1);
  } else {
    Serial.println("SGP30 found :)");
  }

  Serial.println(F("ENV Unit(SHT30 and BMP280) test..."));
  while (!bme.begin(0x76)){  
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    M5.Lcd.println("Could not find a valid BMP280 sensor, check wiring!");
  }


  WiFi.begin(ssid, password);  //  Wi-Fi APに接続
  while (WiFi.status() != WL_CONNECTED) {  //  Wi-Fi AP接続待ち
      delay(100);
  }

  Serial.print("WiFi connected\r\nIP address: ");
  Serial.println(WiFi.localIP());

  ambient.begin(channelId, writeKey, &client);  //  チャネルIDとライトキーを指定してAmbientの初期化
    
  
  Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);

  M5.Lcd.drawString("Initialization...", 40, 200, 4);




}

void loop() {

  
  while(i > 0) {    
    if(millis()- last_millis > 1000) {
      last_millis = millis();
      i--;
      M5.Lcd.fillRect(10, 200, 40, 20, TFT_BLUE);
      M5.Lcd.drawNumber(i, 10, 200, 4);
    }
  }
  M5.Lcd.fillRect(0, 200, 300, 30, TFT_BLACK);

  if (! sgp.IAQmeasure()) {
    Serial.println("Measurement failed");
    return;
  }

  M5.Lcd.fillRect(100, 40, 220, 70, TFT_BLACK);

  M5.Lcd.drawString("TVOC:",   10, 40, 4);
  M5.Lcd.setTextDatum(TR_DATUM);  // top_right
  M5.Lcd.drawNumber(sgp.TVOC, 220, 40 , 4);
  M5.Lcd.setTextDatum(TL_DATUM);  // top_left
  M5.Lcd.drawString("ppb",    240, 40, 4);

  M5.Lcd.drawString("eCO2:",   10, 60, 4);

  /*
  //eCO2 Color
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  if(sgp.eCO2>600){
     M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
  }
  if(sgp.eCO2>1000){
     M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  }
  if(sgp.eCO2>2000){
     M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
  }
  */
  
  M5.Lcd.setTextDatum(TR_DATUM);  // top_right
  //M5.Lcd.drawNumber(sgp.eCO2, 220, 60, 6);
  M5.Lcd.drawNumber(sgp.eCO2, 220, 60, 4);
  M5.Lcd.setTextDatum(TL_DATUM);  // top_left
  M5.Lcd.drawString("ppm",    240, 60, 4);

  
  Serial.print("TVOC:"); Serial.print(sgp.TVOC); Serial.print(",");
  Serial.print("eCO2:"); Serial.print(sgp.eCO2); Serial.print(",");

  //ENV2
  pressure = bme.readPressure();
  if(sht30.get()==0){
    tmp = sht30.cTemp;
    hum = sht30.humidity;
  }
  Serial.printf("Temp:%2.2f,Hume:%0.2f,Press:%0.2f\n", tmp, hum, pressure/100);

  char buf[60];
  /*
  //Temp Color
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  if(tmp>3.0){
     M5.Lcd.setTextColor(TFT_BLUE, TFT_BLACK);
  }
  if(tmp>10.0){
     M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
  }
  if(tmp>20.0){
     M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  }
  if(tmp>30.0){
     M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
  }
  */

  sprintf(buf,"Temp : %2.2f *C",tmp);
  M5.Lcd.drawString(buf,   10, 120, 4);


  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);

  sprintf(buf,"Hume : %0.2f %%",hum);
  M5.Lcd.drawString(buf,   10, 150, 4);

  sprintf(buf,"Press: %0.2f hPa",pressure/100);
  M5.Lcd.drawString(buf,   10, 170, 4);

  
  //M5.Lcd.printf("Temp: %2.1f  \r\nHumi: %2.0f%%  \r\nPressure:%2.0fPa\r\n", tmp, hum, pressure);


 
  delay(1000);

  // 60秒に一回
  if(millis()- last_Anvi_millis> 60000) {
    last_Anvi_millis = millis();

      // Ambientデータセット
      ambient.set(1, String(sgp.eCO2).c_str());
      ambient.set(2, String(sgp.TVOC).c_str());
      
      ambient.set(3, String(tmp).c_str());
      ambient.set(4, String(hum).c_str());
      ambient.set(5, String(pressure/100).c_str());
      
      // Ambient送出
      ambient.send();

  }

  
}
