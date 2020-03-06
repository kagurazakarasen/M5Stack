#include <M5Stack.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <string.h>

//const char* fname = "/wifi.csv"; // 同名で定義済みなのでコメントアウト
File fp;
char ssid[32];
char pass[32];

void SetwifiSD(const char *file){
  unsigned int cnt = 0;
  char data[64];
  char *str;

  fp = SD.open(file, FILE_READ); // fname だったのを file に変更  
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
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  M5.Lcd.print("IP: ");
  M5.Lcd.println(WiFi.localIP());
  Serial.printf("IP: ");
  Serial.println(WiFi.localIP());
  fp.close();
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
