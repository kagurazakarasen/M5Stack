#include <M5Stack.h>
#include <Wire.h>
#include <SparkFunCCS811.h> //Click here to get the library: http://librarymanager/All#SparkFun_CCS811

#define CCS811_ADDR 0x5B //Default I2C Address

//Global sensor objects
CCS811 myCCS811(CCS811_ADDR);

void setup()
{

  M5.begin();
  //M5.Power.begin();
  Wire.begin();

  //Serial.begin(115200);
  Serial.println();
  //Serial.println("Apply BME280 data to CCS811 for compensation.");
  Serial.println("CCS811 data to M5Stack.");


  //This begins the CCS811 sensor and prints error status of .beginWithStatus()
  CCS811Core::CCS811_Status_e returnCode = myCCS811.beginWithStatus();
  Serial.print("CCS811 begin exited with: ");
  Serial.println(myCCS811.statusString(returnCode));

    //for M5Stack 
    M5.Lcd.setBrightness(10);
}
//---------------------------------------------------------------
void loop()
{

    M5.Lcd.clear();



  //Check to see if data is available
  if (myCCS811.dataAvailable())
  {


    //Calling this function updates the global tVOC and eCO2 variables
    myCCS811.readAlgorithmResults();
    //printInfoSerial fetches the values of tVOC and eCO2
    printInfoSerial();

  
  }
  else if (myCCS811.checkForStatusError())
  {
    //If the CCS811 found an internal error, print it.
    printSensorError();
  }
  //for M5Stack 
  delay(10000); //Wait for next reading
}

//---------------------------------------------------------------
void printInfoSerial()
{

  uint16_t co2;
  uint16_t tvoc;

  //getCO2() gets the previously read data from the library
  Serial.println("CCS811 data:");
  Serial.print(" CO2 concentration : ");
  co2=myCCS811.getCO2();
  Serial.print(co2);
  Serial.println(" ppm");

  // for M5Stack
  unsigned long sec = millis() / 1000;

  if(sec >= 1200) {
    M5.Lcd.setTextColor(TFT_WHITE);
  } else {
    M5.Lcd.setTextColor(TFT_BLUE);
  }
  
  if(co2>999){
    M5.Lcd.setTextColor(TFT_YELLOW);
  }
  if(co2>1999){
    M5.Lcd.setTextColor(TFT_RED);
  }

  //M5.Lcd.plintln(co2);
  //M5.Lcd.plintln(ppm);  


    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextSize(3);
    M5.Lcd.print("CO2 concentration");  
    M5.Lcd.setCursor(220, 70);
    //M5.Lcd.setTextSize(3);
    M5.Lcd.print("ppm");  


  M5.Lcd.setCursor(40, 50);
  M5.Lcd.setTextSize(7);
  M5.Lcd.print(co2);
  M5.Lcd.print(" ");



  //getTVOC() gets the previously read data from the library
  tvoc=myCCS811.getTVOC();
  Serial.print(" TVOC concentration : ");
  Serial.print(tvoc);
  Serial.println(" ppb");


  M5.Lcd.setCursor(100, 150);
  M5.Lcd.setTextSize(5);
  M5.Lcd.print(tvoc);
  M5.Lcd.print(" ");

 

    M5.Lcd.setCursor(10, 165);
    M5.Lcd.setTextSize(3);
    M5.Lcd.print("TVOC");

    M5.Lcd.setCursor(220, 165);
    M5.Lcd.print("ppb");


}

//printSensorError gets, clears, then prints the errors
//saved within the error register.
void printSensorError()
{
  uint8_t error = myCCS811.getErrorRegister();

  if (error == 0xFF) //comm error
  {
    Serial.println("Failed to get ERROR_ID register.");
  }
  else
  {
    Serial.print("Error: ");
    if (error & 1 << 5)
      Serial.print("HeaterSupply");
    if (error & 1 << 4)
      Serial.print("HeaterFault");
    if (error & 1 << 3)
      Serial.print("MaxResistance");
    if (error & 1 << 2)
      Serial.print("MeasModeInvalid");
    if (error & 1 << 1)
      Serial.print("ReadRegInvalid");
    if (error & 1 << 0)
      Serial.print("MsgInvalid");
    Serial.println();
  }
}
