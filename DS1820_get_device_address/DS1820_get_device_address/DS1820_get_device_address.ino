#include <OneWire.h> //Temperature
#include <DallasTemperature.h> //Temperature

#define ONE_WIRE_BUS 8
#define TEMPERATURE_PRECISION 12

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);  
  sensors.begin();
  int countSensors = sensors.getDeviceCount();
  
  DeviceAddress address;
  for (int i = 0; i < countSensors; i++) {
    
    sensors.getAddress(address, i);
    Serial.println(convertAddressToString(address));
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}

/* got from https://handyman.dulare.com/esp8266-ds18b20-web-server/ */
String convertAddressToString(DeviceAddress deviceAddress) {
  String addrToReturn = "";
  for (uint8_t i = 0; i < 8; i++){
    if (deviceAddress[i] < 16) addrToReturn += "0";
    addrToReturn += String(deviceAddress[i], HEX);
  }
  return addrToReturn;
}