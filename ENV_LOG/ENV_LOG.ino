/*
 * M5StackとENV UNITを使った環境ロガー
 * https://github.com/kagurazakarasen/M5Stack
 */

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

//保存ログファイルの上限(最大99)
#define FILE_LOG_MAX 5

// 保存するファイル名
char log_fname[20];
//char* log_fname = "/env_log.csv";
#define  LOG_fnameHead  "/envlog"
#define  LOG_fnameExt  ".csv"
int8_t LogF_Cnt = 0;

// ループのウェイト、何秒待つかをミリ秒で指定
unsigned long int DELAY = 1000;    // ミリ秒

//何秒に一度SDカードにログを書き込むか（そのタイミングで取得したデータのみ） 
unsigned int LOG_WRITE_RATE = 60;  // （秒）↓が割り切れる値にしてね。
unsigned int LOG_WRITE_RATE_COUNT = 1; // DELAY/1000 * LOG_WRITE_RATE の値。

const char* WiFiFile = "/wifi.csv";


//表示用色設定
#define TEMP_COLOR  YELLOW
#define HUME_COLOR  GREEN
#define PRES_COLOR  LIGHTGREY
#define BATT_COLOR  RED

#define G_FLAME_COLOR  DARKGREY


//グラフの描画レンジ設定
float TempMin = -10.0;
float TempMax = 40.0;

int HumMin =0;
int HumMax =100;

int PresMin = 950;
int PresMax =1050;


//グラフ用
int16_t px = 0; // 表示用x座標
int16_t pty = 120; // 温度表示用y座標
int16_t phy = 120; // 温度表示用y座標
int16_t ppy = 120; // 温度表示用y座標

// スクリーンセーバー用カウンタ
#define SCC_MAX 100
int16_t scc = SCC_MAX;


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

    M5.Lcd.println("ENV Unit test...");

    // ENV Unit Check
    Serial.println(F("ENV Unit(DHT12 and BMP280) test..."));

    while (!bme.begin(0x76)){  
      Serial.println("Could not find a valid BMP280 sensor, check wiring!");
      M5.Lcd.println("Could not find a valid BMP280 sensor, check wiring!");
    }

    //ログファイル名セット
    sprintf(log_fname,"%s%02d%s",LOG_fnameHead,LogF_Cnt,LOG_fnameExt);
    M5.Lcd.printf("logFile:%s",log_fname);
    SD.remove(log_fname); //まず消しておく（appendされてしまうので）

    delay(1000);

     
    // ログ書き込み用カウンタ
    LOG_WRITE_RATE_COUNT = DELAY/1000 * LOG_WRITE_RATE;
    if(LOG_WRITE_RATE_COUNT<1)LOG_WRITE_RATE_COUNT=1;

    // LCD初期化
    M5.Lcd.clear(BLACK);

   // M5.Lcd.drawLine(0,0,320,240,BLUE); //表示枠確認テスト

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
  file = SD.open(log_fname, FILE_APPEND);
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


    //グラフプロット用時間（X軸）セット
    long tx =3600*timeinfo.tm_hour + 60*timeinfo.tm_min+timeinfo.tm_sec;
    px = (int)(tx / 270);

    //グラフ最大最小値表示
    M5.Lcd.setTextSize(1);  // 文字サイズ
    M5.Lcd.setCursor(0, 110); // カーソル
    M5.Lcd.setTextColor(TEMP_COLOR, BLACK);  // 色
    M5.Lcd.printf("%2.1f", TempMax);
    M5.Lcd.setTextColor( G_FLAME_COLOR, BLACK);  // 色
    M5.Lcd.printf("/");
    M5.Lcd.setTextColor(HUME_COLOR, BLACK);  // 色
    M5.Lcd.printf("%d%%", HumMax);
    M5.Lcd.setTextColor( G_FLAME_COLOR, BLACK);  // 色
    M5.Lcd.printf("/");
    M5.Lcd.setTextColor(PRES_COLOR, BLACK);  // 色
    M5.Lcd.printf("%dhPa", PresMax);

    M5.Lcd.setCursor(0, 230); // カーソル
    M5.Lcd.setTextColor(TEMP_COLOR, BLACK);  // 色
    M5.Lcd.printf("%2.1f", TempMin);
    M5.Lcd.setTextColor( G_FLAME_COLOR, BLACK);  // 色
    M5.Lcd.printf("/");
    M5.Lcd.setTextColor(HUME_COLOR, BLACK);  // 色
    M5.Lcd.printf("%d%%", HumMin);
    M5.Lcd.setTextColor( G_FLAME_COLOR, BLACK);  // 色
    M5.Lcd.printf("/");
    M5.Lcd.setTextColor(PRES_COLOR, BLACK);  // 色
    M5.Lcd.printf("%dhPa", PresMin);


    //グラフ表示(Y=110～240)の範囲でプロット
    //気温
    float TmpRangeDelta = 130.0/(TempMax - TempMin);
    float tmpY = TmpRangeDelta * tmp;
    pty = 240+(int)(TempMin*TmpRangeDelta)-(int)tmpY;

    //湿度
    float HumRangeDelta = 130.0/(HumMax - HumMin);
    float HumY = HumRangeDelta * hum;
    //phy = 210 - (int)(hum); // 0-100%なので
    phy = 240+(int)(HumMin*HumRangeDelta)-(int)HumY;

    //気圧
    float PPreDelta = 130.0/(PresMax - PresMin);
    float preY = PPreDelta * (float)(pressure);

    //ppy = 240 - (int)(pressure - 950);
    ppy = 240+(int)( PresMin*PPreDelta) - (int)(preY);

    //pty = 120;

    // まずライン消去
    M5.Lcd.drawLine(px+1,240,px+1,110,BLACK); 

    //グラフ枠プロット
    M5.Lcd.drawRect(0, 110, 320, 240, G_FLAME_COLOR); //なぜか下のラインが出ない
    M5.Lcd.drawLine(0,239,320,239,G_FLAME_COLOR); //下ライン

    M5.Lcd.drawLine(160,110,160,240,G_FLAME_COLOR); //12時
    M5.Lcd.drawLine(80,110,80,240,G_FLAME_COLOR); //6時
    M5.Lcd.drawLine(240,110,240,240,G_FLAME_COLOR); //18時


    //各ポイントをプロット
    M5.Lcd.drawPixel(px, pty, TEMP_COLOR );
    M5.Lcd.drawPixel(px, phy, HUME_COLOR);
    M5.Lcd.drawPixel(px, ppy, PRES_COLOR);
    
    M5.Lcd.fillTriangle(px-5, 100, px+5, 100, px, 110, WHITE);
    M5.Lcd.drawLine(px-5-1,100,px-1, 115, BLACK);


    
    // 日付切り替わり    
    if(px>319){
      px=0;
      M5.Lcd.fillTriangle(319-6, 100, 319, 100, 319, 110, BLACK);
      M5.Lcd.drawLine(319,240,319,110,BLACK); // ライン消去

      //ログファイルローテーション処理
      LogF_Cnt++;
      if(LogF_Cnt>FILE_LOG_MAX){
        LogF_Cnt=0;
      }
      //ログファイル名セット
      sprintf(log_fname,"%s%02d%s",LOG_fnameHead,LogF_Cnt,LOG_fnameExt);
      M5.Lcd.printf("SetlogFile:%s",log_fname);
      SD.remove(log_fname); //まず消しておく（appendされてしまうので）

      
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


    // スクリーンセーバー処理
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


    // ボタンイベント処理
    //Bボタンを押したらキャリブレーションする
    if (M5.BtnB.wasPressed()) {
      M5.Lcd.setBrightness(100);
      scc=SCC_MAX;
      
      //M5.Lcd.printf("now:%2.1f/H:%2.1f/L:%2.1f",tmp,tmp+10.0,tmp-10.0);
      TempMin = tmp-15.0;
      TempMax = tmp+15.0;

      HumMin =hum-10;
      HumMax =hum+10;

      PresMin = pressure - 20;
      PresMax = pressure + 20;
      
    }

    
    
    M5.update();
    
    delay(DELAY);
} 
