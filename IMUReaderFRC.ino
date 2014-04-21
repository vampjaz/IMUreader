/*
Custom IMU reader rig for Arduino
By            Jackson Servheen
created       1-18-2014
modified      4-10-2014
credits       ADXL345 code from bildr tutorial - http://bildr.org/2011/03/adxl345-arduino/
              HMC5883L code from sparkfun tutorial - https://www.sparkfun.com/tutorials/301
              Rest by me
license       GNU GPL v2
requirements  ADXL345 library from tutorial above
              OneWire library from arduino playground - http://playground.arduino.cc/Learning/OneWire
              Time, DS1307RTC libraries from http://playground.arduino.cc/Code/time
connections   A4,A5 -- I2C on ADXL, HMC, and DS1307
              A2,A3 -- temp and gyro reading
              D7    -- oneWire to temperature sensors
              D10-D13- SD card for datalogger
              D0,D1 -- serial connection to cRio
notes         you will have to modify the adxl345 library to use address 0x1D
*/

#include <SPI.h>
#include <Wire.h>
#include <ADXL345.h>
#include <OneWire.h>
#include <SD.h>
#include <Time.h>  
#include <Wire.h>  
#include <DS1307RTC.h>

#define NUM_TEMP_SENSORS 4

ADXL345 adxl; //variable adxl is an instance of the ADXL345 library
OneWire  ds(7);  // on pin 7 (a 4.7K resistor is necessary)

void setup(){
 Serial.begin(9600);
 adxl.powerOn();
 
//Put the HMC5883 IC into the correct operating mode
  Wire.beginTransmission(0x1E); //open communication with HMC5883
  Wire.write(0x02); //select mode register
  Wire.write(0x00); //continuous measurement mode
  Wire.endTransmission();
 
 pinMode(10, OUTPUT);

  if (!SD.begin(10)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
   // return;
  }

  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  if(timeStatus()!= timeSet) 
     Serial.println("Unable to sync with the RTC");
  else
     Serial.println("RTC has set the system time"); 
}

void loop(){
 // Read all the sensors------------------------
 //accelerometer stuff  
 int ax,ay,az; 
 adxl.readAccel(&ax, &ay, &az); //read the accelerometer values and store them in variables x,y,z

 //magnetometer stuff
 int mx,my,mz;
 
 Wire.beginTransmission(0x1E);
  Wire.write(0x03); //select register 3, X MSB register
  Wire.endTransmission();
  
 
 //Read data from each axis, 2 registers per axis
  Wire.requestFrom(0x1E, 6);
  if(6<=Wire.available()){
    mx = Wire.read()<<8; //X msb
    mx |= Wire.read(); //X lsb
    mz = Wire.read()<<8; //Z msb
    mz |= Wire.read(); //Z lsb
    my = Wire.read()<<8; //Y msb
    my |= Wire.read(); //Y lsb
  }
  
  int gyro, gtmp;   //variables for temp and value from Gyroscope
  
  gyro = analogRead(3);
  gtmp = analogRead(2);
  
  int aax,aay;    //variables for analog accellerometer
  
  aax = analogRead(1);
  aay = analogRead(0);
  /*
  float tempc[NUM_TEMP_SENSORS];
  
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  
  for (int t = 0; t < NUM_TEMP_SENSORS; t++) {
    
    if ( !ds.search(addr)) {
    ds.reset_search();
    delay(250);
    return;
  }
  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      //return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad
    
    for (int i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  tempc[t] = (float)raw / 16.0;
  }*/

  // read clock:

  int hr,tmin,sec,yr,mon,tday;

  hr = hour();
  tmin = minute();
  sec = second();

  yr = year();
  mon = month();
  tday = day();
  
 //Done reading sensors-----------------------------
 
  //Print out values of each sensor
    // Output x,y,z values
   Serial.print("Accel: ");
   Serial.print(ax);
   Serial.print(", ");
   Serial.print(ay);
   Serial.print(", ");
   Serial.print(az);
   
   Serial.print("\tAccel 2: ");
   Serial.print(aax);
   Serial.print(", ");
   Serial.print(aay);
   
  Serial.print("\tMagneto: ");
  Serial.print(mx);
  Serial.print(", ");
  Serial.print(my);
  Serial.print(", ");
  Serial.print(mz);
  
  Serial.print("\tGyro: ");
  Serial.print(gyro);
  
  Serial.print(" Gyro Temp: ");
  Serial.print(gtmp);
  
 /* Serial.print(" Temperature Sensors: ");
  for (int a = 0; a < NUM_TEMP_SENSORS; a++) {
    Serial.print(tempc[a]);
    Serial.print(", ");
  }*/

  Serial.print("\tAt: ");
  Serial.print(hr);
  Serial.print(':');
  Serial.print(tmin);
  Serial.print(':');
  Serial.print(sec);

  Serial.print("\tOn: ");
  Serial.print(mon);
  Serial.print('/');
  Serial.print(tday);
  Serial.print('/');
  Serial.print(yr);

  Serial.println();
  
  /*
  //print to datafile -------
   File dataFile = SD.open("frclog.csv", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    
   dataFile.print(ax);
   dataFile.print(',');
   dataFile.print(ay);
   dataFile.print(',');
   dataFile.print(az);
   dataFile.print(',');
   dataFile.print(mx);
   dataFile.print(',');
   dataFile.print(my);
   dataFile.print(',');
   dataFile.print(mz);
   dataFile.print(',');
   dataFile.print(gyro);
   dataFile.print(',');
   dataFile.print(gtmp);
   dataFile.print(',');
   dataFile.print(aax);
   dataFile.print(',');
   dataFile.print(aay);
   dataFile.print(',');
   for (int a = 0; a < NUM_TEMP_SENSORS; a++) {
    //dataFile.print(tempc[a]);
    dataFile.print(',');
   }
  dataFile.print(hr);
  dataFile.print(',');
  dataFile.print(tmin);
  dataFile.print(',');
  dataFile.print(sec);

  dataFile.print(',');
  dataFile.print(mon);
  dataFile.print(',');
  dataFile.print(tday);
  dataFile.print(',');
  dataFile.print(yr);
   dataFile.println();
    dataFile.close();
 
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog");
  }*/
  
  delay(150);
}

