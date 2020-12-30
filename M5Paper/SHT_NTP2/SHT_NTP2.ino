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

int lineNum=0;

M5EPD_Canvas canvas(&M5.EPD);

void WiFi_setup()
{
      //connect to WiFi
        //WiFi.begin(ssid, password); // 最初はこちらで接続。
        WiFi.begin();                 // 一度接続が成功したらこちらでOK.
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
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


void setup()
{
    M5.begin();
    M5.SHT30.Begin();
    //Serial.begin(115200);
    M5.EPD.SetRotation(90);
    M5.EPD.Clear(true);
    canvas.createCanvas(540,960);
    canvas.setTextSize(2);

    //WiFi
    WiFi_setup();
    NTP_setup();
    WiFI_off();
}

void loop()
{
    lineNum++;
    if(lineNum>960/16) lineNum=0;

    //TimeCheck(NTP)
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  YY = timeinfo.tm_year+1900;
  MM = timeinfo.tm_mon+1;
  DD = timeinfo.tm_mday;
    
  hh = timeinfo.tm_hour;
  mm = timeinfo.tm_min;
  ss = timeinfo.tm_sec;

  Serial.printf("%2d %2d %2d",hh,mm,ss);

    M5.SHT30.UpdateData();
    tem = M5.SHT30.GetTemperature();
    hum = M5.SHT30.GetRelHumidity();
    Serial.printf("Temperatura: %2.2f*C  Humedad: %0.2f%%\r\n", tem, hum);
    dtostrf(tem, 2, 2 , temStr);
    dtostrf(hum, 2, 2 , humStr);
    canvas.drawString(
     String(YY-208) + "/" + String(MM) + "/" + String(DD) + " " +
     String(hh) + ":" + String(mm) + ":" + String(ss) +
     " Temp:" + String(temStr) + "C " + "Hume:" + String(humStr), 10, lineNum*16);
   // canvas.drawString("Hume:" + String(humStr) , 100, 200);
    //canvas.pushCanvas(0,300,UPDATE_MODE_A2);
    canvas.pushCanvas(0,0,UPDATE_MODE_A2);
    delay(60000); // 一分待機（あとでスリープ処理すること）
}

// Function to extract numbers from compile time string
static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}
