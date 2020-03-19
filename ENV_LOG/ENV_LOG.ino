/*
 * M5StackとENV UNITを使った環境ロガーです
 * https://github.com/kagurazakarasen/M5Stack
 * 
 * 制作日記的なモノ：https://note.com/rasen/n/naa51e575b0e4
 * 
 * ＠神楽坂らせん
 */

#include <M5Stack.h>
#include "DHT12.h"
#include <Wire.h> //The DHT12 uses I2C comunication.
#include "Adafruit_Sensor.h"
#include <Adafruit_BMP280.h>
#include <WiFi.h>
#include <time.h>
#include <string.h>
#include <HTTPClient.h>

// ファイル保存するかどうか。ファイルに保存するならここを true にする
boolean FILEWRITE = true;

//保存ログファイルの上限(最大99)
#define FILE_LOG_MAX 30

// 保存するファイル名
char log_fname[20];
#define  LOG_fnameHead  "/envlog"
#define  LOG_fnameExt  ".csv"
int8_t LogF_Cnt = 0;

// ループのウェイト、何秒待つかをミリ秒で指定
unsigned long int DELAY = 1000;    // ミリ秒

//何秒に一度SDカードにログを書き込むか（そのタイミングで取得したデータのみ） 
unsigned int LOG_WRITE_RATE = 60;  // （秒）↓が割り切れる値にしてね。
unsigned int LOG_WRITE_RATE_COUNT = 1; // DELAY/1000 * LOG_WRITE_RATE の値。

//WiFi設定ファイル名
const char* WiFiFile = "/wifi.csv";


/*
// Slack にログをポストするかどうか
boolean SLACK_POST = true;

//SlackのhookURL入りのファイル
const char* Slackfname = "/slackhook.txt";


const char *server = "hooks.slack.com";
char *json = "{\"text\":\"ENV_LOG START\",\"icon_emoji\":\":ghost:\",\"username\":\"m5stackpost\"}";

const char* slack_root_ca= \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
"QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
"CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
"nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
"43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
"T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
"gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
"BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
"TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
"DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
"hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
"06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
"PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
"YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
"CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \
"-----END CERTIFICATE-----\n" ;
   
HTTPClient http;

String services;
*/

//表示用色設定
#define TEMP_COLOR  YELLOW
#define HUME_COLOR  GREEN
#define PRES_COLOR  LIGHTGREY
#define BATT_COLOR  RED

#define G_FLAME_COLOR  DARKGREY


//グラフの描画レンジ初期設定
float TempMin = -10.0;
float TempMax = 40.0;
float HumMin =0.0;
float HumMax =100.0;
float PresMin = 950.0;
float PresMax =1050.0;

//ボタン操作で変化するグラフの縦幅
#define TempRangeChangeSize 10.0
#define PresRangeChangeSize 10.0

float Temp_ARRAY[321];
float Hume_ARRAY[321];
float Press_ARRAY[321];

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
/*
void slack_init(){
    //Slack
    unsigned int cnt = 0;
    char data[128];
    char *str;
    File fp;



    fp = SD.open(Slackfname, FILE_READ);
    if(fp == false){  SLACK_POST = false;  }
    
    if(SLACK_POST){
        while(fp.available()){
          data[cnt++] = fp.read();
        }
        close(fp);
    
        String s = (String)data;
    
        int l = s.indexOf("/services/");
        //M5.Lcd.println(l);  // 23のはず
      
        //M5.Lcd.print(s.substring(l));
      
        //String services = s.substring(l);
        services = s.substring(l);
        M5.Lcd.print(services);
    
        // Slack Post
        http.begin( server, 443, services, slack_root_ca );
        http.addHeader("Content-Type", "application/json" );
        http.POST((uint8_t*)json, strlen(json));
        M5.Lcd.println("post hooks.slack.com");
        http.end();
    }  

}
*/

/*
void slack_post(char *json_buf){
        //Slack Post
        if( SLACK_POST ){
            //sprintf(jsonnow,"{\"text\":\"Temperatura: %2.2f*C  Humedad: %0.2f%%  Pressure: %0.2fhPa\",\"icon_emoji\":\":ghost:\",\"username\":\"m5stackpost\"}", tmp, hum, pressure);
            Serial.printf(json_buf);
        
            // Slack Post
            http.begin( server, 443, services, slack_root_ca );
            http.addHeader("Content-Type", "application/json" );
            http.POST((uint8_t*)json_buf, strlen(json_buf));
            Serial.printf("\n post hooks.slack.com\n");
            http.end();
        }

}
*/

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



    //Slack
    slack_init();
    char *json = "{\"text\":\"Slack Send START\",\"icon_emoji\":\":ghost:\",\"username\":\"m5stackpost\"}";
    slack_post(json);

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

   //ARRAY初期化
   int i=0;
   while (i<321){
      Temp_ARRAY[i]=-100.0; // プロットエリア外に出す
      Hume_ARRAY[i]=-100.0;
      Press_ARRAY[i]=-100.0;
/*    // DYMMY     
      Temp_ARRAY[i]=24.0;
      Hume_ARRAY[i]=17;
      Press_ARRAY[i]=1000;
*/
      i++;
   }

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
    char jsonnow[255];
    

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
    M5.Lcd.printf("Temp:%2.1fC", tmp);

    M5.Lcd.setCursor(0, 80); // カーソル
    M5.Lcd.setTextColor(HUME_COLOR, BLACK);  // 色
    M5.Lcd.printf("Humi:%2.1f%%", hum);

    M5.Lcd.setCursor(140, 60); // カーソル
    M5.Lcd.setTextColor(PRES_COLOR, BLACK);  // 色
    M5.Lcd.printf("Pres:%2.2fhPa", pressure);

    M5.Lcd.setCursor(140, 80); // カーソル
    M5.Lcd.setTextColor(BATT_COLOR, BLACK);  // 色
    M5.Lcd.printf("Batt: %d%%",batt);


    //グラフプロット用時間（X軸）セット
    long tx =3600*timeinfo.tm_hour + 60*timeinfo.tm_min+timeinfo.tm_sec;
    px = (int)(tx / 270);

    //配列に保存
    Temp_ARRAY[px] = tmp;
    Hume_ARRAY[px] = hum;
    Press_ARRAY[px] = pressure;


    //グラフ最大最小値表示
    M5.Lcd.setTextSize(1);  // 文字サイズ
    M5.Lcd.setCursor(0, 110); // カーソル
    M5.Lcd.setTextColor( G_FLAME_COLOR, BLACK);  // 色
    M5.Lcd.printf("Hi:");
    M5.Lcd.setTextColor(TEMP_COLOR, BLACK);  // 色
    M5.Lcd.printf("%2.1fC", TempMax);
    M5.Lcd.setTextColor( G_FLAME_COLOR, BLACK);  // 色
    M5.Lcd.printf("/");
    M5.Lcd.setTextColor(HUME_COLOR, BLACK);  // 色
    M5.Lcd.printf("%2.0f%%", HumMax);
    M5.Lcd.setTextColor( G_FLAME_COLOR, BLACK);  // 色
    M5.Lcd.printf("/");
    M5.Lcd.setTextColor(PRES_COLOR, BLACK);  // 色
    M5.Lcd.printf("%2.0fhPa", PresMax);

    M5.Lcd.setCursor(0, 230); // カーソル
    M5.Lcd.setTextColor( G_FLAME_COLOR, BLACK);  // 色
    M5.Lcd.printf("Lo:");
    M5.Lcd.setTextColor(TEMP_COLOR, BLACK);  // 色
    M5.Lcd.printf("%2.1fC", TempMin);
    M5.Lcd.setTextColor( G_FLAME_COLOR, BLACK);  // 色
    M5.Lcd.printf("/");
    M5.Lcd.setTextColor(HUME_COLOR, BLACK);  // 色
    M5.Lcd.printf("%2.0f%%", HumMin);
    M5.Lcd.setTextColor( G_FLAME_COLOR, BLACK);  // 色
    M5.Lcd.printf("/");
    M5.Lcd.setTextColor(PRES_COLOR, BLACK);  // 色
    M5.Lcd.printf("%2.0fhPa", PresMin);

    //昨日のデータ表示
    if(Hume_ARRAY[px+1]>-100){
      M5.Lcd.setTextSize(1);
      M5.Lcd.setCursor(160, 230); // カーソル
      M5.Lcd.setTextColor( G_FLAME_COLOR, BLACK);  // 色
      M5.Lcd.printf("lastday:");
      M5.Lcd.setTextColor(TEMP_COLOR, BLACK);  // 色
      M5.Lcd.printf("%2.1fC", Temp_ARRAY[px+1]);
      M5.Lcd.setTextColor( G_FLAME_COLOR, BLACK);  // 色
      M5.Lcd.printf("/");
      M5.Lcd.setTextColor(HUME_COLOR, BLACK);  // 色
      M5.Lcd.printf("%2.0f%%", Hume_ARRAY[px+1]);
      M5.Lcd.setTextColor( G_FLAME_COLOR, BLACK);  // 色
      M5.Lcd.printf("/");
      M5.Lcd.setTextColor(PRES_COLOR, BLACK);  // 色
      M5.Lcd.printf("%2.0fhPa", Press_ARRAY[px+1]);
    }

    //グラフ表示(Y=110～240)の範囲でプロット
    //気温
    float TmpRangeDelta = 130.0/(TempMax - TempMin);
    float tmpY = TmpRangeDelta * tmp;
    //pty = 240+(int)(TempMin*TmpRangeDelta)-(int)tmpY;
    pty = 240.0+ (TempMin*TmpRangeDelta)-tmpY;

    //湿度
    float HumRangeDelta = 130.0/(HumMax - HumMin);
    float HumY = HumRangeDelta * hum;
    //phy = 210 - (int)(hum); // 0-100%なので
    //phy = 240+(int)(HumMin*HumRangeDelta)-(int)HumY;
    phy = 240.0+(HumMin*HumRangeDelta)-HumY;

    //気圧
    float PPreDelta = 130.0/(PresMax - PresMin);
    float preY = PPreDelta * (float)(pressure);

    //ppy = 240 - (int)(pressure - 950);
    //ppy = 240+(int)( PresMin*PPreDelta) - (int)(preY);
    ppy = 240.0+( PresMin*PPreDelta) - (preY);

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

          sprintf(buff,"%2.1f ,%2.1f% ,%2.2f, %d%", tmp, hum, pressure,batt);
          writeData(buff);

          // ログ書き込み用カウンタリセット
          LOG_WRITE_RATE_COUNT = DELAY/1000 * LOG_WRITE_RATE;
          if(LOG_WRITE_RATE_COUNT<1)LOG_WRITE_RATE_COUNT=1;

          //Slack用json作成  
          sprintf(jsonnow,"{\"text\":\"%04d/%02d/%02d  %02d:%02d:%02d  Temperature: %2.2f*C  Humidity: %0.2f%%  Pressure: %0.2fhPa\",\"icon_emoji\":\":ghost:\",\"username\":\"m5stackpost\"}", \
                          timeinfo.tm_year + 1900,timeinfo.tm_mon + 1,timeinfo.tm_mday, \
                          timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, \
                          tmp, hum, pressure);

          slack_post(jsonnow);
         
       }
    }


    // スクリーンセーバー処理
    scc--;
    if(scc<1){
      scc=SCC_MAX;
      M5.Lcd.setBrightness(10);
    }
    
    // ボタンイベント処理
    boolean btn_on_flg = false;
    //Aボタンを押したら上限引き上げ
    if (M5.BtnA.wasPressed()) {
      btn_on_flg = true;
      
      //TempMin = tmp-15.0;
      TempMax = TempMax+ TempRangeChangeSize;

      //HumMin =hum-10;
      //HumMax =HumMax+10;

      //PresMin = pressure - 20;
      PresMax = PresMax + PresRangeChangeSize;
    }


    //Bボタンを押したら中央値にキャリブレーションする
    if (M5.BtnB.wasPressed()) {
      btn_on_flg = true;
      
      TempMin = tmp - TempRangeChangeSize;
      TempMax = tmp + TempRangeChangeSize;

      //HumMin = hum - 20;
      //HumMax = hum + 20;
      HumMin = 0.0;   // 湿度は変化させない
      HumMax = 100.0;

      PresMin = pressure - PresRangeChangeSize + 2.0; //Tempと重なるので＋２ずらしておく
      PresMax = pressure + PresRangeChangeSize + 2.0; //Tempと重なるので＋２ずらしておく

    }

    //Cボタンを押したら下限引き下げ
    if (M5.BtnC.wasPressed()) {
      btn_on_flg = true;
      
      TempMin = TempMin-TempRangeChangeSize;
      //TempMax = tmp+15.0;

      //HumMin =HumMin-10;
      //HumMax =hum+10;

      PresMin = PresMin - PresRangeChangeSize;
      //PresMax = pressure + 20;

    }

    if(btn_on_flg){
      TmpRangeDelta = 130.0/(TempMax - TempMin);
      HumRangeDelta = 130.0/(HumMax - HumMin);
      PPreDelta = 130.0/(PresMax - PresMin);
      
      //枠内消去
      M5.Lcd.fillRect(0, 100, 320, 240, BLACK);

      int i=0;
      while (i<320){

          /*
          pty = 240+(int)(TempMin*TmpRangeDelta)-(int)(TmpRangeDelta * Temp_ARRAY[i]); //気温
          phy = 240+(int)(HumMin*HumRangeDelta)-(int)(HumRangeDelta * Hume_ARRAY[i]); //湿度
          ppy = 240+(int)( PresMin*PPreDelta) - (int)(PPreDelta * Press_ARRAY[i]); //気圧
          */

          pty = 240.0+(TempMin*TmpRangeDelta)-(TmpRangeDelta * Temp_ARRAY[i]); //気温
          phy = 240.0+(HumMin*HumRangeDelta)-(HumRangeDelta * Hume_ARRAY[i]); //湿度
          ppy = 240.0+( PresMin*PPreDelta) - (PPreDelta * Press_ARRAY[i]); //気圧

          //各ポイントをプロット
          M5.Lcd.drawPixel(i, pty, TEMP_COLOR );
          M5.Lcd.drawPixel(i, phy, HUME_COLOR);
          M5.Lcd.drawPixel(i, ppy, PRES_COLOR);
        i++;
      }

      M5.Lcd.setBrightness(100);
      scc=SCC_MAX;
      btn_on_flg=false;
    }
    
    M5.update();
    
    delay(DELAY);
} 
