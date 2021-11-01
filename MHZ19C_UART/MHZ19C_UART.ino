/* source: https://arduino-technology.ru/coding/hardware/mhz19b-sensor/
 *  Add 
 *    MHZ19_uart.cpp
 *    MHZ19_uart.h
 *  to scketch folder
 *  
 *  only this scketch works for my MHZ19C. 
 *  Other scketches show 700ppm on fresh air which is abnormal
 *  */
#include <Arduino.h>
#include "MHZ19_uart.h"
 
MHZ19_uart mhz19;
 
void setup()
{
  int status;
 
  Serial.begin(9600);
 
  mhz19.begin(10, 11);
  mhz19.setAutoCalibration(true);
  
  status = mhz19.getStatus();
  Serial.println(status);
  delay(2000);
  
  status = mhz19.getStatus();
  Serial.println(status);
  delay(2000);
}
 
void loop()
{
  Serial.println(mhz19.getPPM());
  delay(10000);
}
