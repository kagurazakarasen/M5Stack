/* 
 *  『M5StackのWiFIクライアント設定をSDカードから読み込んで利用する』@kmaepu
 *  https://qiita.com/kmaepu/items/c390d80973efa316ca4a　より。
 *　基本は生かしてENV_LOG用に改修　＠ 神楽坂らせん
 */

/*
#include <M5Stack.h>
#include <WiFi.h>
*/

#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <string.h>

//const char* fname = "/wifi.csv"; // 同名で定義済みなのでコメントアウト
File fp;
char ssid[32];
char pass[32];

boolean SetwifiSD(const char *file){  // タイムアウト確認用に戻り値を設定
  unsigned int cnt = 0;
  char data[64];
  char *str;

  fp = SD.open(file, FILE_READ); // fname だったのを file に変更  
  if(fp != true){                // エラー処理
    M5.Lcd.printf("WiFi FileOpenError!!");
    delay(1000);
    return false;
  }
  
  while(fp.available()){
    data[cnt++] = fp.read();
  }
  strtok(data,",");
  str = strtok(NULL,"\r");    // CR
  strncpy(&ssid[0], str, strlen(str));

  strtok(NULL,",");
  str = strtok(NULL,"\r");    // CR
  strncpy(&pass[0], str, strlen(str));

  M5.Lcd.printf("WIFI-SSID: %s\n",ssid);
  M5.Lcd.printf("WIFI-PASS: %s\n",pass);
  M5.Lcd.println("Connecting...");

  Serial.printf("SSID = %s\n",ssid);
  Serial.printf("PASS = %s\n",pass);

  // STA設定
  WiFi.mode(WIFI_STA);     // STAモードで動作
  WiFi.begin(ssid, pass);
  unsigned int failCnt = 0; // 失敗用カウンタ
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      failCnt++;
      if(failCnt> 50){
        M5.Lcd.printf("WiFi Timed Out!!");
        delay(1000);

        return false;
      }
  }

  M5.Lcd.print("IP: ");
  M5.Lcd.println(WiFi.localIP());
  Serial.printf("IP: ");
  Serial.println(WiFi.localIP());
  fp.close();

  return true;
}

/*
void setup() {
  M5.begin();
  M5.Lcd.setTextSize(2);

  SetwifiSD(fname); // Get ssid
}

void loop() {
  // put your main code here, to run repeatedly:
}
*/
