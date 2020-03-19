/*
 * M5Stack用、水位チェックプログラム
 * 水センサーはプルアップしておき、G26につなぐこと。
 * 
 * M5Stack_WiFi_SD.ino と、 SlackSend.ino を同じディレクトリに入れておくこと。 
 * 
 */

//#include <M5StickC.h>
#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>

//WiFi設定ファイル名
const char* WiFiFile = "/wifi.csv";

// デジタル入力PIN
int D_IN_PIN = 26;  // プルアップされるので無入力で１


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



// the setup routine runs once when M5StickC starts up
void setup(){
  // Initialize the M5StickC object
  M5.begin();
   M5.Power.begin();
   
    M5.Lcd.println("WiFi begin");

    if(SetwifiSD(WiFiFile)){
      M5.Lcd.println("Connect!");
      //M5.Speaker.tone(661, 200); 
      //M5.update();
    }else{
      M5.Lcd.println("No Connect!");            
    }

  slack_init();
  slack_post("{\"text\":\"電源ON、監視開始します！\",\"icon_emoji\":\":ghost:\",\"username\":\"水ボトル監視君\"}");
  
  pinMode(D_IN_PIN, INPUT);
  // LCD display

  M5.Lcd.setRotation(1);  // ボタンBが上になる向き
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE, BLACK); 
  M5.Lcd.setTextSize(2);


  M5.Speaker.setBeep(900, 500);
  //M5.Speaker.tone(661, 10);
  M5.Lcd.setTextColor(BLUE, BLACK); 
  M5.Lcd.print("Water Cheaker");
    M5.Lcd.fillScreen(BLACK);

}

void pw_off(){
      M5.Lcd.setTextSize(3);
      M5.Lcd.setCursor(00, 150);
      M5.Lcd.setTextColor(TFT_DARKGREY, RED); 
      M5.Lcd.printf("OK Power off ! \r\n");
      M5.Speaker.end();
      slack_post("{\"text\":\"電源offします。さようなら～\",\"icon_emoji\":\":ghost:\",\"username\":\"水ボトル監視君\"}");
      delay(600);
      M5.powerOFF();
  
}

bool BtFlg = false;

// the loop routine runs over and over again forever
void loop() {

  
    uint8_t batt = getBattery();
    char sbuf[500];


    //M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
  
    //M5.Lcd.print("<- Double Click Shutdonwn\n\nWater Cheaker\n\n");
    M5.Lcd.setTextColor(BLUE, BLACK); 
    M5.Lcd.setTextSize(4);
    M5.Lcd.print("WATER CHECKER\n");

    M5.Lcd.setTextColor(WHITE, BLACK); 
    M5.Lcd.setTextSize(2);
    M5.Lcd.print("Please put on sensor \n   in the bottle.\n");



    int water_flg;
    water_flg = digitalRead(D_IN_PIN);
    
    Serial.printf("%d\n", water_flg );

    M5.Lcd.setCursor(0, 100);
    M5.Lcd.setTextSize(2);
    M5.Lcd.printf("Now Status : ");

    //M5.Lcd.printf("On Water:%d", water_flg * (-1));

    M5.Lcd.setTextSize(4);

    if( water_flg == 0 ){ // 0だったら水が来ている
      M5.Lcd.setTextColor(BLUE, BLACK); 
      M5.Lcd.printf("WET ! ");
      //slack_post("{\"text\":\"水がたまりました！\",\"icon_emoji\":\":ghost:\",\"username\":\"m5stackpost\"}");
      slack_post("{\"text\":\"水がたまりました！\",\"icon_emoji\":\":ghost:\",\"username\":\"水ボトル監視君\"}");
      M5.Speaker.beep(); //beep
      delay(1000);

    }else{
      M5.Lcd.setTextColor(TFT_ORANGE, BLACK); 
      M5.Lcd.printf("DRY ! ");
          //sprintf(sbuf,"{\"text\":\"Time Chk -- Batt: %d%%\",\"icon_emoji\":\":ghost:\",\"username\":\"m5stackpost\"}",batt);
          //slack_post(sbuf);
    }


    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(60, 160);
    M5.Lcd.setTextColor(RED, BLACK); 
    M5.Lcd.printf("Battery: %d%%  ",batt);
    //バッテリーが少なくなったら？
    if(batt == 25 ){
        if(BtFlg == false){
            slack_post("{\"text\":\"そろそろバッテリーが切れそう！充電してください！\",\"icon_emoji\":\":ghost:\",\"username\":\"水ボトル監視君\"}");
            BtFlg=true;
        } // 一度だけ
    }

    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, 200);
    M5.Lcd.setTextColor(TFT_DARKGREY, BLACK); 
    M5.Lcd.printf("     POWER OFF BUTTON\n");
    M5.Lcd.printf("     V      V      V");


    //ボタン監視
    if(M5.BtnA.wasPressed()) {
      pw_off();
    }
    if(M5.BtnB.wasPressed()) {
      pw_off();
    }
    if(M5.BtnC.wasPressed()) {
      pw_off();
    }

 
    delay(500);
    //delay(10000);
    M5.update();
  
}
