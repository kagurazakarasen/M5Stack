// MoStack CoreInk用、 NTPサーバから日時を取得、RTCを使って15秒ごとに時刻を表示
// RTC系に CoreInkライブラリ 0.0.2 以降が必要

#include "M5CoreInk.h"
#include <WiFi.h>
#include <time.h>

const String SSID = "";  // ← WiFiのSSIDを入れる
const String WF_KEY = "";// ← WiFiのパスキーを入れる

// 日付のゲット＆セット用のライブラリ内のTypo対策用。（0.0.1= Data / 0.0.2 = Date)
#define GetData GetDate
#define SetData SetDate


static const char *wd[7] = {"Sun","Mon","Tue","Wed","Thr","Fri","Sat"};

#define JST     3600*9

tm timeinfo;
time_t now;

Ink_Sprite InkPageSprite(&M5.M5Ink);

RTC_TimeTypeDef RTCtime;
RTC_DateTypeDef RTCDate;

char timeStrbuff[64];

void flushTime(){
    M5.rtc.GetTime(&RTCtime);
    M5.rtc.GetData(&RTCDate); // 0.0.1ライブラリ用
    //M5.rtc.GetDate(&RTCDate); // 0.0.2 ライブラリはこちら
    
    sprintf(timeStrbuff,"%d/%02d/%02d %02d:%02d:%02d",
                        RTCDate.Year,RTCDate.Month,RTCDate.Date,
                        RTCtime.Hours,RTCtime.Minutes,RTCtime.Seconds);
                                         
    
    InkPageSprite.drawString(20,120,timeStrbuff,&AsciiFont8x16);


    sprintf(timeStrbuff,"%02d:%02d", RTCtime.Hours,RTCtime.Minutes);

    InkPageSprite.drawString(30,50,timeStrbuff,&AsciiFont24x48);
    
    InkPageSprite.pushSprite();

    showTime(timeinfo);
}

 
void setupTime(){

  RTCtime.Minutes = timeinfo.tm_min;
  RTCtime.Seconds = timeinfo.tm_sec;
  RTCtime.Hours = timeinfo.tm_hour;
  RTCDate.Year = timeinfo.tm_year+1900;
  RTCDate.Month = timeinfo.tm_mon+1;
  RTCDate.Date = timeinfo.tm_mday;
  RTCDate.WeekDay = timeinfo.tm_wday;
  
  M5.rtc.SetTime(&RTCtime);
  M5.rtc.SetData(&RTCDate);

  char timeStrbuff[64];
  sprintf(timeStrbuff, "%d/%02d/%02d %02d:%02d:%02d",
            RTCDate.Year, RTCDate.Month, RTCDate.Date,
            RTCtime.Hours, RTCtime.Minutes, RTCtime.Seconds);

  Serial.println("[NTP] in: " + String(timeStrbuff));

}

void setup_wifi(){
    // Connect to an access point
    if(SSID.length() == 0 ){
      // SSIDの設定を最初にしていなかったら、      
      WiFi.begin();                 // Connect to the access point of the last connection
    } else {
      // SSIDに何か文字列が入っていたら
      Serial.print("Connect to :");
      Serial.println( SSID );
      char SSIDbuff[64];
      char KeyBuff[64];
      sprintf(SSIDbuff,"%s",SSID);
      sprintf(KeyBuff,"%s",WF_KEY);
      WiFi.begin(SSIDbuff, KeyBuff);  // Or, Connect to the specified access point
    }

    //
  
    Serial.print("Connecting to Wi-Fi ");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println(" CONNECTED");
 
}

void showTime(tm localTime) {
    //Serial.print("[NTP] ");
    Serial.print(localTime.tm_mday);
    Serial.print('/');
    Serial.print(localTime.tm_mon + 1);
    Serial.print('/');
    Serial.print(localTime.tm_year - 100);
    Serial.print('(');
        Serial.print(wd[localTime.tm_wday]);
    Serial.print(")-");

    Serial.print(localTime.tm_hour);
    Serial.print(':');
    Serial.print(localTime.tm_min);
    Serial.print(':');
    Serial.println(localTime.tm_sec);
}

bool getNTPtime(int sec) {
    {
        Serial.print("[NTP] sync.");
        uint32_t start = millis();
        do {
            time(&now);
            localtime_r(&now, &timeinfo);
            Serial.print(".");
            delay(10);
        } while (((millis() - start) <= (1000 * sec)) && (timeinfo.tm_year < (2016 - 1900)));
        if (timeinfo.tm_year <= (2016 - 1900)) return false;  // the NTP call was not successful

        Serial.print("now ");
        Serial.println(now);
        //saveRtcData();
        setupTime();

        char time_output[30];
        strftime(time_output, 30, "%a  %d-%m-%y %T", localtime(&now));
        Serial.print("[NTP] ");
        Serial.println(time_output);
    }
    return true;
}


void setup() {

    M5.begin();
    Serial.begin(115200);
    delay(50);

    setup_wifi();

    // Set ntp time to local
    //configTime(9 * 3600, 0, ntpServer);
    configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

    if (getNTPtime(10)) {  // wait up to 10sec to sync
        } else {
            Serial.println("[NTP] Time not set");
            ESP.restart();
        }
      showTime(timeinfo);
      /* ブザー：五月蠅いので止めておく
      M5.Speaker.tone(2700,200);
      delay(100);
      M5.Speaker.mute();
      */

    if( !M5.M5Ink.isInit())
    {
        Serial.printf("Ink Init faild");
        while (1) delay(100);   
    }
    M5.M5Ink.clear();
    delay(1000);
    //creat ink refresh Sprite
    if( InkPageSprite.creatSprite(0,0,200,200,true) != 0 )
    {
        Serial.printf("Ink Sprite creat faild");
    }
    setupTime();
}

void loop() {
  flushTime();
  delay(15000);     // 15秒 ホールド
  M5.shutdown(45);  // 45秒 スリープ　USBがPCに刺さっている（電源が入っている）と機能せず。また、再起時に完全に初期状態から起動してしまう（setupから始まり、NTP読み込みもする。（しない場合はRTCがリセットされてしまう。うーむ）
}
