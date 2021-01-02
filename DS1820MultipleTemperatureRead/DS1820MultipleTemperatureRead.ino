#include <SPI.h> //Ethernet shield
#include <Ethernet.h> //Ethernet shield
#include <PubSubClient.h> //MQTT

#include <Arduino_JSON.h>

#include <OneWire.h> //Temperature
#include <DallasTemperature.h> //Temperature

#define ONE_WIRE_BUS A5
#define TEMPERATURE_PRECISION 12

byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};
byte server[] = { 192, 168, 1, 21 };

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

EthernetClient ethClient;
PubSubClient client(ethClient);

void setup()
{    
  Serial.begin(9600);  
  Serial.println("Setup started");
  //sensors.begin();      
  initializeEthernet();
  initializeMQTT();
  Serial.println("Setup finished");
  
  delay(500);//Wait for newly restarted system to stabilize
}

void initializeMQTT(){
  Serial.println("MQTT initialize started");
  client.setServer(server, 1883);
  if (client.connect("/heat/temperature")) {
    client.publish("/heat/temperature", "OK");    
    Serial.println("MQTT Working");
    //client.subscribe("/myhome/in/#");
  }
  else{
    Serial.println("MQTT not connected");
  }
  Serial.println("MQTT initialize finished");
}

void initializeEthernet(){
  Serial.println("Ethernet initialize started");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }    
  }else{
    Serial.print("My IP address: ");
    Serial.println(Ethernet.localIP());  
  }  
  Serial.println("Ethernet initialize finished");
}

void callback(char* topic, byte* payload, unsigned int length) {
  /*payload[length] = '\0';
  Serial.print(topic);
  Serial.print("  ");
  String strTopic = String(topic);
  String strPayload = String((char*)payload);
  Serial.println(strPayload);

  if (strTopic == "/myhome/in/Kitchen_light1") {
    if (strPayload == "OFF")     digitalWrite(light1_pin, LOW);
    else if (strPayload == "ON") digitalWrite(light1_pin, HIGH);
  } 
  else if (strTopic == "/myhome/in/Kitchen_light2") {
    if (strPayload == "OFF") digitalWrite(light2_pin, LOW);
    else if (strPayload == "ON") digitalWrite(light2_pin, HIGH);
  }*/ 
}

void loop(void)
{
  /*JSONVar result = checkThermometers();  
  Serial.println(JSON.stringify(result));  
  Serial.println("DONE");
  delay(1000);*/
}

JSONVar checkThermometers(){
  int totalThermometers = sensors.getDeviceCount();
  oneWire.reset_search();
  DeviceAddress currentThermometer;  
  JSONVar result;
  result["totalThermometers"] = totalThermometers;
  JSONVar thermometersJson;
  sensors.requestTemperatures(); 
  for (int i = 0;  i < totalThermometers;  i++) {
    JSONVar thermometer;
    sensors.getAddress(currentThermometer,i);
    thermometer["address"] = convertAddressToString(currentThermometer);
    thermometer["temperatureC"] = sensors.getTempC(currentThermometer);
    thermometersJson[i]=thermometer;
  }
  result["thermometers"] = thermometersJson;
  return result;
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
