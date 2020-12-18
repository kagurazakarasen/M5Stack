#include "M5CoreInk.h"
// #include <M5Stack.h>
#include <Wire.h>
#include "Adafruit_Sensor.h"
#include <Adafruit_BMP280.h>
#include "SHT3X.h"
SHT3X sht30;
Adafruit_BMP280 bme;

float tmp = 0.0;
float hum = 0.0;
float pressure = 0.0;

Ink_Sprite InkPageSprite(&M5.M5Ink);

RTC_TimeTypeDef RTCtime;
RTC_DateTypeDef RTCDate;

char timeStrbuff[64];
char tempStrbuff[64];
char pressStrbuff[64];

void flushTime(){
    M5.rtc.GetTime(&RTCtime);
    M5.rtc.GetData(&RTCDate);
    
  
  pressure = bme.readPressure();
  if(sht30.get()==0){
    tmp = sht30.cTemp;
    hum = sht30.humidity;
  }
  sprintf(tempStrbuff,"%2.2f*C : %0.2f%% ", tmp, hum);
  sprintf(pressStrbuff,"%0.2fPa", pressure);
  
  
  sprintf(timeStrbuff,"%d/%02d/%02d %02d:%02d:%02d",
                        RTCDate.Year,RTCDate.Month,RTCDate.Date,
                        RTCtime.Hours,RTCtime.Minutes,RTCtime.Seconds);


                                         
    InkPageSprite.drawString(10,50,timeStrbuff);
    InkPageSprite.drawString(10,100,tempStrbuff);
    InkPageSprite.drawString(10,130,pressStrbuff);
    InkPageSprite.pushSprite();
}

void setupTime(){
  
  RTCtime.Hours = 23;
  RTCtime.Minutes = 33;
  RTCtime.Seconds = 33;
  M5.rtc.SetTime(&RTCtime);
  
  RTCDate.Year = 2020;
  RTCDate.Month = 11;
  RTCDate.Date = 6;
  M5.rtc.SetData(&RTCDate);

  Serial.println(F("ENV Unit(SHT30 and BMP280) test..."));

  while (!bme.begin(0x76)){  
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    //M5.Lcd.println("Could not find a valid BMP280 sensor, check wiring!");
  }
  
}

void setup() {

    M5.begin();
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

  
  delay(15000);
}
