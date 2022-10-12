#include <SPI.h> //Ethernet shield
#include <Ethernet.h> //Ethernet shield
#include <MHZ.h> //MHZ19C
#include <PubSubClient.h> //MQTT
#include <OneWire.h> //Temperature
#include <DallasTemperature.h> //Temperature
/*
 *  Add 
 *    MHZ19_uart.cpp
 *    MHZ19_uart.h
 *  to scketch folder
 */
 
#include <Arduino.h>
#define CO2_IN 3
#define ONE_WIRE_BUS 8
#define TEMPERATURE_PRECISION 12
#define bataryController  7
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
byte server[] = { 192, 168, 1, 180 };

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

EthernetClient ethClient;
PubSubClient client(ethClient);
MHZ co2(CO2_IN, MHZ19C);
char* monitorConnectedTopic = "/heat/temperatureMonitor";
String thermometersCountTopic = "/heat/thermometersCount";
String thermometersAddressTopic = "/heat/thermometersAddress";
String thermometerTmpTopic = "/heat/termometer1";
String bataryControllerTopic = "/heat/batary1";
char* bataryActionTopic = "/heat/batary1/turn";
String thermometersInfoTopicStart = "/heat/";
String arduinoAliveTopic = "/alive/arduino/";
String roomThermometerAddress = "28ab9707b6013c6e";
String co2Topic = "/gaz/co2";
float roomThermometerValue = 0;

bool bataryIsHot = true;
void setup()
{    
  Serial.begin(9600);  
  Serial.println("Setup started");
  sensors.begin();      
  initializeEthernet();
  initializeMQTT();
  initializeMHZ19();
  pinMode(bataryController, OUTPUT);
  Serial.println("Setup finished");  
  delay(500);//Wait for newly restarted system to stabilize
}

void initializeMHZ19(){
  pinMode(CO2_IN, INPUT);
  delay(100);
  Serial.println("MHZ 19B");

  // enable debug to get addition information
  // co2.setDebug(true);

  if (co2.isPreHeating()) {
    Serial.print("Preheating");
    while (co2.isPreHeating()) {
      Serial.print(".");
      delay(5000);
    }
    Serial.println();
    }
}

void initializeMQTT(){
  Serial.println("MQTT initialize started");
  client.setServer(server, 8083 );
  client.setCallback(callback);
  
  if (client.connect(monitorConnectedTopic)) {
    sendMQTTMessage(monitorConnectedTopic, "OK");    
    Serial.println("MQTT Working");
  }
  else{
    Serial.println("MQTT not connected");
  }
  client.subscribe("/heat/batary1/turn");
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
  /*Example of receiving commands from MQTT*/
  Serial.println("Message received");
  payload[length] = '\0';
  Serial.print(topic);
  Serial.print("  ");
  String strTopic = String(topic);
  String strPayload = String((char*)payload);
  Serial.println(strPayload);
  if (strTopic == bataryActionTopic) {
    if (strPayload == "1")     switchBataryToHot(true);
    else if (strPayload == "0") switchBataryToHot(false);
  }  
}

void loop(void)
{
  checkConnections();
  client.loop();
  checkThermometers();   
  checkCo2Sensor(); 
  checkBatary();
  delay(10000);
}

void checkConnections(){
  if (!ethClient.connected()){
    initializeEthernet();
    initializeMQTT();
  }
  if (!client.connected()){
    initializeMQTT();
  } 
  sendMQTTMessage(arduinoAliveTopic, "OK");
}

void sendMQTTMessage(String topic, String message){
  Serial.println("to MQTT: " + topic + ": " + message);  
  client.publish(topic.c_str(),message.c_str());    
}

void switchBataryToHot(bool setHot){
  if (setHot && !bataryIsHot){
    digitalWrite(bataryController, HIGH);
    bataryIsHot = true;
    sendMQTTMessage(bataryControllerTopic, String(bataryIsHot));
  }
  if (!setHot && bataryIsHot){
    digitalWrite(bataryController, LOW);
    bataryIsHot = false;
    sendMQTTMessage(bataryControllerTopic, String(bataryIsHot));
  }
}

void checkThermometers(){
  int totalThermometers = sensors.getDeviceCount();
  oneWire.reset_search();
  DeviceAddress currentThermometer;  
  sendMQTTMessage(thermometersCountTopic, String(totalThermometers));
  String allAddress = "";
  sensors.requestTemperatures(); 
  for (int i = 0;  i < totalThermometers;  i++) {    
    sensors.getAddress(currentThermometer,i);
    String address = convertAddressToString(currentThermometer);
    float currentThermometerValue = sensors.getTempC(currentThermometer);    
    if (address == "28ab9707b6013c6e") {
      roomThermometerValue = currentThermometerValue;
      sendMQTTMessage(thermometerTmpTopic, String(currentThermometerValue));          
    }
    allAddress = allAddress + address + " ";    
    sendMQTTMessage(thermometersInfoTopicStart + address, String(currentThermometerValue));    
  }  
  sendMQTTMessage(thermometersAddressTopic, allAddress);
  
}

void checkBatary(){
  if (roomThermometerValue < 18){
    switchBataryToHot(true);
  }
  if (roomThermometerValue > 26){
    switchBataryToHot(false);
  }
  sendMQTTMessage(bataryControllerTopic, String(bataryIsHot));
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

void checkCo2Sensor(){
  sendMQTTMessage(co2Topic, String(co2.readCO2PWM()));
}