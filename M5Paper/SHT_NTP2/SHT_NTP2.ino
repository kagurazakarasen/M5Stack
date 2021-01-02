#include <M5EPD.h>

#include <WiFi.h>
#include "time.h"

const char* ssid       = "";  // SSIDとキー、自分の環境のを入れておく
const char* password   = "";

const char* ntpServer =  "ntp.jst.mfeed.ad.jp";
const long  gmtOffset_sec = 9 * 3600;
const int   daylightOffset_sec = 0;

static uint8_t conv2d(const char* p); // Forward declaration needed for IDE 1.6.x

uint8_t hh = conv2d(__TIME__), mm = conv2d(__TIME__ + 3), ss = conv2d(__TIME__ + 6); // Get H, M, S from compile time
uint8_t YY,MM,DD;

char temStr[10];
char humStr[10];

float tem;
float hum;

//int lineNum=0;

File f,f2;

M5EPD_Canvas canvas(&M5.EPD);

#define SLEEP_SEC 60    // スリープ時間（秒） 5秒以上にすること。

void WiFi_setup()
{
  int i = 0;
      //connect to WiFi
        //WiFi.begin(ssid, password); // 最初はこちらで接続。
        WiFi.begin();                 // 一度接続が成功したらこちらでOK.
        while (WiFi.status() != WL_CONNECTED) {
            i++;
            delay(500);
            Serial.print(".");
            if(i>50){
              Serial.println("TimeOut");
              return;
            }
        }
        Serial.println("CONNECTED");
}

void NTP_setup()
{
      //init and get the time
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
            Serial.println("Failed to obtain time");
            return;
        }
}

void WiFI_off()
{
      //disconnect WiFi as it's no longer needed
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}

// WakeUp FLG set
void f2set(int flg)
{
  // Open log file
  f2 = SD.open("/wake.txt", FILE_WRITE );
  if (!f2) {
    Serial.printf("wake flag FILE is NOT OPEN.");
  } else {
    if(flg){
      Serial.printf("SetSD to %d\n",flg);
      f2.println(flg);
    }else{
      Serial.printf("SetSD to 0\n");
      f2.println("0");
    }
  }
  f2.close();
}

int f2get()
{
  String strBuf;
  
  f2 = SD.open("/wake.txt", FILE_READ );
  if (!f2) {
    Serial.printf("Wake FILE is NOT READ.");
    return 0;
  } else {

  /* サイズ分ループ */
    while(f2.available())
    {
      strBuf = strBuf + f2.readString();
    }
    /* ファイルクローズ */   
    f2.close();
    Serial.printf("Read from SD %s\n",strBuf);

    if( strBuf.toInt()>0){
      return strBuf.toInt();
    }else{
      return 0;
    }

  }
  f2.close();
  return 0;

}

void SD_init(){
  // Start SD card
  if (!SD.begin()) {
    Serial.println("ERROR: SD CARD.");
    canvas.drawString("NO SD CARD", 10, 100);
    canvas.pushCanvas(0,0,UPDATE_MODE_A2);
  }

}

void setup()
{

    int lineNum; // ライン数カウンタ、WakeUpのチェックに使用
    M5.begin();
//   M5.Power.begin();// M5.power関係つかえず＞＜
    M5.SHT30.Begin();
    //Serial.begin(115200);
    M5.EPD.SetRotation(90);
    //M5.EPD.Clear(true);
    canvas.createCanvas(540,960);
    canvas.setTextSize(2);

  SD_init();
  
  lineNum=f2get();
  if(lineNum<1){
    M5.EPD.Clear(true);
    canvas.drawString("ColdStart---", 0, 0);
    canvas.pushCanvas(0,0,UPDATE_MODE_DU4);

  }else{

    canvas.drawString("OnLoopStart---" + String(lineNum) , 0, 0);
    canvas.pushCanvas(0,0,UPDATE_MODE_DU4);
  }

    //WiFi
    WiFi_setup();
    NTP_setup();
    WiFI_off();
}

int Chk_battery()
{
  char buf[20];
  uint32_t vol = M5.getBatteryVoltage();
  if(vol < 3300)
    {
        vol = 3300;
    }
    else if(vol > 4350)
    {
        vol = 4350;
    }
    float battery = (float)(vol - 3300) / (float)(4350 - 3300);
    if(battery <= 0.01)
    {
        battery = 0.01;
    }
    if(battery > 1)
    {
        battery = 1;
    }
    //uint8_t px = battery * 25;
    sprintf(buf, "BATT%d%% ", (int)(battery * 100));
    canvas.drawString(buf , 450, 0);

    return (int)(battery * 100);
}

void loop()
{
    int lineNum =  f2get();
    lineNum++;
    if(lineNum>960/16){
      lineNum=1;
      M5.EPD.Clear(true);
    }
    f2set(lineNum);

    int btLevel = Chk_battery();

    //TimeCheck(NTP)
  bool NTP_ON = true;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    NTP_ON = false;
  } else {
    YY = timeinfo.tm_year+1900;
    MM = timeinfo.tm_mon+1;
    DD = timeinfo.tm_mday;
      
    hh = timeinfo.tm_hour;
    mm = timeinfo.tm_min;
    ss = timeinfo.tm_sec;

    Serial.printf("%2d %2d %2d",hh,mm,ss);
  }

    // SHT30
    M5.SHT30.UpdateData();
    tem = M5.SHT30.GetTemperature();
    hum = M5.SHT30.GetRelHumidity();
    Serial.printf("Temperatura: %2.2f*C  Humedad: %0.2f%%\r\n", tem, hum);
    dtostrf(tem, 2, 2 , temStr);
    dtostrf(hum, 2, 2 , humStr);

    String lineStr;
    lineStr = "";
    if(NTP_ON){
        lineStr = String(YY-208) + "/" + String(MM) + "/" + String(DD) + " " +
        String(hh) + ":" + String(mm) + ":" + String(ss);
    } else {
        lineStr = "Meybe +" + String(SLEEP_SEC)  + "sec after.";
    }
    lineStr = lineStr + " Temp:" + String(temStr) + "C " + "Hume:" + String(humStr) + " ";
    lineStr = lineStr + String(btLevel) + " ";
       
    canvas.drawString(lineStr , 10, lineNum*16);
    canvas.pushCanvas(0,0,UPDATE_MODE_A2);

  // Open log file
  f = SD.open("/testLog.txt", FILE_APPEND );
  if (!f) {
    Serial.printf("LOG FILE is NOT OPEN.");
  } else {
    f.println(lineStr);
  }
    
    //delay(1000);
    // 一分待機 
    delay(5000);  // ５秒
    if(btLevel==1){ // 電源につないでいるとバッテリーレベルが１になるので、それを利用して判定
      //delay(55000);  // バッテリー接続時にはディレイを＋55秒とする
      delay( (SLEEP_SEC - 5)*1000 );
    }
    //M5.Power.lightSleep(SLEEP_SEC(5)); // だめ。使えない
    //M5.shutdown(55);  // （電源無接続時のみ有効。電源ついてるかチェックしたいけれどやりかたわかんない＞＜）
    M5.shutdown( SLEEP_SEC - 5 );
}

// Function to extract numbers from compile time string
static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}
