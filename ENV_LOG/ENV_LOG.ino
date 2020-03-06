#include <M5Stack.h>
#include "DHT12.h"
#include <Wire.h> //The DHT12 uses I2C comunication.
#include "Adafruit_Sensor.h"
#include <Adafruit_BMP280.h>
#include <WiFi.h>
#include <time.h>
#include <string.h>


// ファイル保存するかどうか。ファイルに保存するならここを true にする
boolean FILEWRITE = true;

// 保存するファイル名
const char* fname = "/env_log.csv";

// ループのウェイト、何秒待つかをミリ秒で指定
unsigned long int DELAY = 1000;    // ミリ秒

//何秒に一度SDカードにログを書き込むか（そのタイミングで取得したデータのみ） 
unsigned int LOG_WRITE_RATE = 60;  // （秒）↓が割り切れる値にしてね。
unsigned int LOG_WRITE_RATE_COUNT = 1; // DELAY/1000 * LOG_WRITE_RATE の値。

const char* WiFiFile = "/wifi.csv";

//グラフの描画レンジ設定
#define TMP_MIN -10.0
#define TMP_MAX 40.0

#define HUM_MIN 0
#define HUM_MAX 100

#define PRS_MIN 950
#define PRS_MAX 1050


// === グローバル変数、定数
DHT12 dht12; //Preset scale CELSIUS and ID 0x5c.
Adafruit_BMP280 bme;

#define TEMP_COLOR  YELLOW
#define HUME_COLOR  GREEN
#define PRES_COLOR  DARKGREY
#define BATT_COLOR  RED

//グラフ用
int16_t px = 0; // 表示用x座標
int16_t pty = 120; // 温度表示用y座標
int16_t phy = 120; // 温度表示用y座標
int16_t ppy = 120; // 温度表示用y座標

// スクリーンセーバー用カウンタ
#define SCC_MAX 100
int16_t scc = SCC_MAX;

// === 関数プロトタイプ宣言
uint8_t getBattery(uint16_t, uint16_t);

// Time
char ntpServer[] = "ntp.jst.mfeed.ad.jp";   // ntpサーバ 
const long gmtOffset_sec = 9 * 3600;
const int  daylightOffset_sec = 0;
struct tm timeinfo;
//String dateStr;
//String timeStr;
char dateS[12];
char timeS[12];

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

    //M5.Lcd.setBrightness(10);
    M5.Lcd.setBrightness(100);

    M5.Lcd.println("WiFi begin");

    if(SetwifiSD(WiFiFile)){
      M5.Lcd.println("Connect!");
      // timeSet
      getTimeFromNTP(); // コネクトしたらNTPを見に行く。（接続できなかったらtimeinfoが0になり、日付が1970/1/1になる）

    }else{
      M5.Lcd.println("No Connect!");            
    }
  
    Wire.begin();
    
    // ENV Unit Check
    Serial.println(F("ENV Unit(DHT12 and BMP280) test..."));

    while (!bme.begin(0x76)){  
      Serial.println("Could not find a valid BMP280 sensor, check wiring!");
      M5.Lcd.println("Could not find a valid BMP280 sensor, check wiring!");
    }
    // LCD初期化
    M5.Lcd.println("ENV Unit test...");
    M5.Lcd.clear(BLACK);

    // ログ書き込み用カウンタ
    LOG_WRITE_RATE_COUNT = DELAY/1000 * LOG_WRITE_RATE;
    if(LOG_WRITE_RATE_COUNT<1)LOG_WRITE_RATE_COUNT=1;

}

void getTime(){
  // 時刻の取得と表示

  //char timeS[12];
  //char dateS[12];
  getLocalTime(&timeinfo);
  
  sprintf(dateS,"%04d/%02d/%02d",timeinfo.tm_year + 1900,timeinfo.tm_mon + 1,timeinfo.tm_mday);
  sprintf(timeS,"%02d:%02d:%02d",timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

  M5.Lcd.setCursor(0, 0); // カーソル
  M5.Lcd.setTextSize(2);  // 文字サイズ
  M5.Lcd.setTextColor(WHITE, BLACK);  // 色
  M5.Lcd.printf("%s\n",dateS);
  //M5.Lcd.println(" ");

  M5.Lcd.setCursor(30, 18); // カーソル
  M5.Lcd.setTextSize(5);  // 文字サイズ
  M5.Lcd.printf("%s",timeS);

  //M5.Lcd.setCursor(0, 0); // カーソル
  //M5.Lcd.setTextSize(2);  // 文字サイズ
  //M5.Lcd.printf("\n\n");


  //dateStr = String(dateS);  //これでいけるけど無駄なので整理
  //timeStr = String(timeS);
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
  file.println((String)dateS + "," + (String)timeS + "," + paramStr);
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


    // 時刻表示
    getTime();
    //M5.Lcd.printf("--------\n");

    //M5.Lcd.setTextSize(2);  // 文字サイズ

    //LCD表示クリア＆色設定
    //M5.Lcd.setCursor(0, 60); // カーソル
    M5.Lcd.setTextColor(WHITE, BLACK);  // 色
    M5.Lcd.setTextSize(2);  // 文字サイズ

    // 温度、湿度、気圧、バッテリー情報
    M5.Lcd.setCursor(0, 60); // カーソル
    M5.Lcd.setTextColor(TEMP_COLOR, BLACK);  // 色
    M5.Lcd.printf("Temp:%2.1f", tmp);

    M5.Lcd.setCursor(0, 80); // カーソル
    M5.Lcd.setTextColor(HUME_COLOR, BLACK);  // 色
    M5.Lcd.printf("Humi:%2.0f%%", hum);

    M5.Lcd.setCursor(140, 60); // カーソル
    M5.Lcd.setTextColor(PRES_COLOR, BLACK);  // 色
    M5.Lcd.printf("Pres:%2.0fhPa", pressure);

    M5.Lcd.setCursor(140, 80); // カーソル
    M5.Lcd.setTextColor(BATT_COLOR, BLACK);  // 色
    M5.Lcd.printf("Batt: %d%%",batt);


    //グラフ表示
    float TmpRangeDelta = 120.0/(TMP_MAX - TMP_MIN);
    float tmpY = TmpRangeDelta * tmp;
    pty = 240-(int)tmpY;

    phy = 210 - (int)(hum); // 0-100%なので


    float PPreDelta = 120.0/(PRS_MAX - PRS_MIN);
    float preY = PPreDelta * (float)(pressure-PRS_MIN);

    //ppy = 240 - (int)(pressure - 950);
    ppy = 240 - (int)(preY);

    //pty = 120;

    M5.Lcd.drawLine(px,240,px,120,BLACK); // まずライン消去
    
    M5.Lcd.drawPixel(px, pty, TEMP_COLOR );
    M5.Lcd.drawPixel(px, phy, HUME_COLOR);
    M5.Lcd.drawPixel(px, ppy, PRES_COLOR);
    
    M5.Lcd.fillTriangle(px-5, 100, px+5, 100, px, 110, WHITE);
    M5.Lcd.drawLine(px-5-1,100,px-1, 115, BLACK);

    //時間
    long tx = timeinfo.tm_hour * 24 +timeinfo.tm_min*60+timeinfo.tm_sec;

//    px++;
    px = (int)(tx / 270);

    M5.Lcd.setCursor(0, 200); // カーソル
    M5.Lcd.setTextSize(1);  // 文字サイズ
    //M5.Lcd.printf("tx:%d\n", tx);
    //M5.Lcd.printf("px:%d\n", px);

    
    
    if(px>319){
      px=0;
      M5.Lcd.fillTriangle(319-6, 100, 319+5, 100, 319, 110, BLACK);
      M5.Lcd.drawLine(319,240,319,110,BLACK); // ライン消去
    }

    //M5.Lcd.printf(":%d",px);
    
    //ログファイル出力
    if (FILEWRITE){
       LOG_WRITE_RATE_COUNT--;  //カウンタをデクリメント
       //M5.Lcd.printf(":%d",LOG_WRITE_RATE_COUNT);
       if( LOG_WRITE_RATE_COUNT < 1 ){
         // M5.Lcd.printf("*");

          sprintf(buff,"%2.1f ,%2.0f% ,%2.0f, %d%", tmp, hum, pressure,batt);
          writeData(buff);

          // ログ書き込み用カウンタリセット
          LOG_WRITE_RATE_COUNT = DELAY/1000 * LOG_WRITE_RATE;
          if(LOG_WRITE_RATE_COUNT<1)LOG_WRITE_RATE_COUNT=1;

       }
    }

    scc--;
    if(scc<1){
      scc=SCC_MAX;
      M5.Lcd.setBrightness(10);
    }
    // ボタンイベント処理
    //Aボタンを押したときに明るくするテスト。今はDELAYの切り替わりのタイミングでしか動作しない
    if (M5.BtnA.wasPressed()) {
      M5.Lcd.setBrightness(100);
      scc=SCC_MAX;
    }
    M5.update();
    
    delay(DELAY);
} 
