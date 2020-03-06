#include <M5Stack.h>
#include "DHT12.h"
#include <Wire.h> //The DHT12 uses I2C comunication.
#include "Adafruit_Sensor.h"
#include <Adafruit_BMP280.h>
#include <WiFi.h>
#include <time.h>


// ファイル保存するかどうか。ファイルに保存するならここを true にする
#define FILEWRITE true

// 保存するファイル名
const char* fname = "/env_log.csv";

// ループのウェイト、何秒待つかをミリ秒で指定
const long DELAY = 60000;    // ミリ秒

// WiFiの設定 （XXXのところを自分のWiFiの設定に）
char ssid[] = "XXXXXXXXXXXXX";
char pass[] = "XXXXXXXXXXXXX";



// === グローバル変数、定数
DHT12 dht12; //Preset scale CELSIUS and ID 0x5c.
Adafruit_BMP280 bme;

// === 関数プロトタイプ宣言
uint8_t getBattery(uint16_t, uint16_t);

// Time
char ntpServer[] = "ntp.jst.mfeed.ad.jp";   // ntpサーバ 
const long gmtOffset_sec = 9 * 3600;
const int  daylightOffset_sec = 0;
struct tm timeinfo;
String dateStr;
String timeStr;

File file;

void getTimeFromNTP(){
  // NTPサーバと時刻を同期
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  while (!getLocalTime(&timeinfo)) {
    delay(1000);
  }
}

void setup() {
    M5.begin();

    M5.Lcd.setBrightness(10);

    M5.Lcd.println("WiFi begin");
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      M5.Lcd.print(".");
    }
    M5.Lcd.println("\nWiFi connected.");
  
    // timeSet
    getTimeFromNTP();

    Wire.begin();
    
    // ENV Unit Check
    Serial.println(F("ENV Unit(DHT12 and BMP280) test..."));

    while (!bme.begin(0x76)){  
      Serial.println("Could not find a valid BMP280 sensor, check wiring!");
      M5.Lcd.println("Could not find a valid BMP280 sensor, check wiring!");
    }
    // LCD初期化
    M5.Lcd.clear(BLACK);
    M5.Lcd.println("ENV Unit test...");
}

void getTime(){
  // 時刻の取得と表示
  getLocalTime(&timeinfo);
  dateStr = (String)(timeinfo.tm_year + 1900)
          + "/" + (String)(timeinfo.tm_mon + 1)
          + "/" + (String)timeinfo.tm_mday;
  timeStr = (String)timeinfo.tm_hour
          + ":" + (String)timeinfo.tm_min
          + ":" + (String)timeinfo.tm_sec;

  //M5.Lcd.setTextColor(WHITE,BLACK);
  //M5.Lcd.setCursor(0, 200, 1);
  M5.Lcd.println(dateStr + "   ");
  M5.Lcd.println(timeStr + "   ");
}


// ================================================================== //
// バッテリ残量取得
// 戻り値 : uint8_t 0 ～ 100(%)
// ================================================================== //
uint8_t getBattery() {
  uint8_t vat = 0xFF;
  Wire.beginTransmission(0x75);
  Wire.write(0x78);                   // 0x78 バッテリ残量取得レジスタアドレスオフセット
  Wire.endTransmission(false);
  if (Wire.requestFrom(0x75, 1)) {
    vat = Wire.read() & 0xF0;         // 下位4ビット 0 マスク
    if      (vat == 0xF0) vat = 0;
    else if (vat == 0xE0) vat = 25;
    else if (vat == 0xC0) vat = 50;
    else if (vat == 0x80) vat = 75;
    else if (vat == 0x00) vat = 100;
    else                  vat = 0xFF;
  } else vat = 0xFF;

  return vat;
}


void writeData(char *paramStr) {
  // SDカードへの書き込み処理（ファイル追加モード）
  // SD.beginはM5.begin内で処理されているので不要
  file = SD.open(fname, FILE_APPEND);
  file.println(dateStr + "," + timeStr + "," + paramStr);
  file.close();
}

void loop() {
    // 温度の取得
    float tmp = dht12.readTemperature();

    // 湿度の取得
    float hum = dht12.readHumidity();

    // 気圧の取得[hPa = Pa * 0.01]
    float pressure = bme.readPressure() * 0.01;

    uint8_t batt = getBattery();

    char buff[128];

    // 温度、湿度、気圧をシリアル通信で送信
    Serial.printf("Temperatura: %2.2f*C  Humedad: %0.2f%%  Pressure: %0.2fPa\r\n", tmp, hum, pressure);

    //LCD表示クリア＆色設定
    M5.Lcd.setCursor(0, 0); // カーソル
    M5.Lcd.setTextColor(WHITE, BLACK);  // 色
    M5.Lcd.setTextSize(4);  // 文字サイズ

    // 時刻表示
    getTime();
    M5.Lcd.printf("--------\n");

    // 温度、湿度、気圧、バッテリー情報
    M5.Lcd.printf("Temp:%2.1f \nHumi:%2.0f%% \nPres:%2.0fhPa \nBatt: %d%%", tmp, hum, pressure,batt);

#if FILEWRITE
    // ファイル出力
    sprintf(buff,"%2.1f ,%2.0f% ,%2.0f, %d%", tmp, hum, pressure,batt);
    writeData(buff);
#endif

    // ボタンイベント処理
    //Aボタンを押したときに明るくするテスト。今はDELAYの切り替わりのタイミングでしか動作しない
    if (M5.BtnA.wasPressed()) {
      M5.Lcd.setBrightness(100);
    } else {
      M5.Lcd.setBrightness(10);
    }
    M5.update();
    
    delay(DELAY);
} 
