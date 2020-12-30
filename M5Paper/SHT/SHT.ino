#include <M5EPD.h>

char temStr[10];
char humStr[10];

float tem;
float hum;

int i=0;


M5EPD_Canvas canvas(&M5.EPD);
void setup()
{
    M5.begin();
    M5.SHT30.Begin();
    M5.EPD.SetRotation(90);
    M5.EPD.Clear(true);
    //canvas.createCanvas(400, 300);
    canvas.createCanvas(540,960);
    canvas.setTextSize(2);
}

void loop()
{
    i++;
    if(i>960/16) i=0;
    M5.SHT30.UpdateData();
    tem = M5.SHT30.GetTemperature();
    hum = M5.SHT30.GetRelHumidity();
    Serial.printf("Temperatura: %2.2f*C  Humedad: %0.2f%%\r\n", tem, hum);
    dtostrf(tem, 2, 2 , temStr);
    dtostrf(hum, 2, 2 , humStr);
    canvas.drawString(String(i) + ": Temp:" + String(temStr) + "C  " + "Hume:" + String(humStr), 10, i*16);
   // canvas.drawString("Hume:" + String(humStr) , 100, 200);
    //canvas.pushCanvas(0,300,UPDATE_MODE_A2);
    canvas.pushCanvas(0,0,UPDATE_MODE_A2);
    delay(1000);
}
