#include <SparkFunBME280.h>
#include <SparkFunCCS811.h>
#include <SPI.h>
#include <LoRa.h>
#include "ArduinoLowPower.h"
#include <ArduinoJson.h>
#define CCS811_ADDR 0x5B //Default I2C Address
//#define CCS811_ADDR 0x5A //Alternate I2C Address

#define PIN_NOT_WAKE 5

int counter = 0;

//Global sensor objects
CCS811 myCCS811(CCS811_ADDR);
BME280 myBME280;
void setup()
{
  Serial.begin(9600);
  if (!LoRa.begin(865E6)) //Change this according to your country's band
  {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("Apply BME280 data to CCS811 for compensation.");

  //This begins the CCS811 sensor and prints error status of .begin()
  CCS811Core::status returnCode = myCCS811.begin();
  Serial.print("CCS811 begin exited with: ");
  //Pass the error code to a function to print the results
  printDriverError( returnCode );
  Serial.println();

  //For I2C, enable the following and disable the SPI section
  myBME280.settings.commInterface = I2C_MODE;
  myBME280.settings.I2CAddress = 0x77;

  //Initialize BME280
  //For I2C, enable the following and disable the SPI section
  myBME280.settings.commInterface = I2C_MODE;
  myBME280.settings.I2CAddress = 0x77;
  myBME280.settings.runMode = 3; //Normal mode
  myBME280.settings.tStandby = 0;
  myBME280.settings.filter = 4;
  myBME280.settings.tempOverSample = 5;
  myBME280.settings.pressOverSample = 5;
  myBME280.settings.humidOverSample = 5;

  //Calling .begin() causes the settings to be loaded
  delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.
  myBME280.begin();


}
//---------------------------------------------------------------
void loop()
{
  //Check to see if data is available
  if (myCCS811.dataAvailable())
  {
    //Calling this function updates the global tVOC and eCO2 variables
    myCCS811.readAlgorithmResults();
    //printInfoSerial fetches the values of tVOC and eCO2
    printInfoSerial();

    float BMEtempC = myBME280.readTempC();
    float BMEhumid = myBME280.readFloatHumidity();

    Serial.print("Applying new values (deg C, %): ");
    Serial.print(BMEtempC);
    Serial.print(",");
    Serial.println(BMEhumid);
    Serial.println();

    //This sends the temperature data to the CCS811
    myCCS811.setEnvironmentalData(BMEhumid, BMEtempC);
  }
  else if (myCCS811.checkForStatusError())
  {
    //If the CCS811 found an internal error, print it.
    printSensorError();
  }

}

//---------------------------------------------------------------
void printInfoSerial()
{
  //Enable this for Debug
  /*
    //getCO2() gets the previously read data from the library
    Serial.println("CCS811 data:");
    Serial.print(" CO2 concentration : ");
    Serial.print(myCCS811.getCO2());
    Serial.println(" ppm");

    //getTVOC() gets the previously read data from the library
    Serial.print(" TVOC concentration : ");
    Serial.print(myCCS811.getTVOC());
    Serial.println(" ppb");

    Serial.println("BME280 data:");
    Serial.print(" Temperature: ");
    Serial.print(myBME280.readTempC(), 2);
    Serial.println(" degrees C");

    Serial.print(" Temperature: ");
    Serial.print(myBME280.readTempF(), 2);
    Serial.println(" degrees F");

    Serial.print(" Pressure: ");
    Serial.print(myBME280.readFloatPressure(), 2);
    Serial.println(" Pa");

    Serial.print(" Altitude: ");
    Serial.print(myBME280.readFloatAltitudeMeters(), 2);
    Serial.println("m");

    Serial.print(" %RH: ");
    Serial.print(myBME280.readFloatHumidity(), 2);
    Serial.println(" %");

  */
 // Converting results to JSON for easy access at the end
  StaticJsonDocument<200> doc;
  doc["CO2"] = myCCS811.getCO2();
  doc["TVOC"] = myCCS811.getTVOC();
  doc["Temp"] = myBME280.readTempC();
  doc["Pressure"] = myBME280.readFloatPressure();
  doc["Altitude"] = myBME280.readFloatAltitudeMeters();
  doc["RH"] = myBME280.readFloatHumidity(); 
  serializeJson(doc, LoRa);
  LoRa.print(counter);
  LoRa.endPacket();

  counter++;
  //delay(5000); //Enable for debug
  LowPower.sleep(15000); // Enable for final code, if you face issues while uploading the new sketch press reset button twice
  // Sleep will help you to save battery
  

}

//printDriverError decodes the CCS811Core::status type and prints the
//type of error to the serial terminal.
//
//Save the return value of any function of type CCS811Core::status, then pass
//to this function to see what the output was.
void printDriverError( CCS811Core::status errorCode )
{
  switch ( errorCode )
  {
    case CCS811Core::SENSOR_SUCCESS:
      Serial.print("SUCCESS");
      break;
    case CCS811Core::SENSOR_ID_ERROR:
      Serial.print("ID_ERROR");
      break;
    case CCS811Core::SENSOR_I2C_ERROR:
      Serial.print("I2C_ERROR");
      break;
    case CCS811Core::SENSOR_INTERNAL_ERROR:
      Serial.print("INTERNAL_ERROR");
      break;
    case CCS811Core::SENSOR_GENERIC_ERROR:
      Serial.print("GENERIC_ERROR");
      break;
    default:
      Serial.print("Unspecified error.");
  }
}

//printSensorError gets, clears, then prints the errors
//saved within the error register.
void printSensorError()
{
  uint8_t error = myCCS811.getErrorRegister();

  if ( error == 0xFF ) //comm error
  {
    Serial.println("Failed to get ERROR_ID register.");
  }
  else
  {
    Serial.print("Error: ");
    if (error & 1 << 5) Serial.print("HeaterSupply");
    if (error & 1 << 4) Serial.print("HeaterFault");
    if (error & 1 << 3) Serial.print("MaxResistance");
    if (error & 1 << 2) Serial.print("MeasModeInvalid");
    if (error & 1 << 1) Serial.print("ReadRegInvalid");
    if (error & 1 << 0) Serial.print("MsgInvalid");
    Serial.println();
  }
}
