#include <SPI.h> //Ethernet shield
#include <Ethernet.h> //Ethernet shield

#include <PubSubClient.h> //MQTT

#include <OneWire.h> //Temperature
#include <DallasTemperature.h> //Temperature

#define ONE_WIRE_BUS 8
#define TEMPERATURE_PRECISION 12
#define bataryController  7
#define pump1 2
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
byte server[] = { 192, 168, 1, 180 };

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

EthernetClient ethClient;
PubSubClient client(ethClient);

char* monitorConnectedTopic = "/heat/temperatureMonitor";
String bataryControllerTopic = "/heat/batary1";
String pump1ControllerTopic = "/heat/pump1";
float roomThermometerValue = 0;

bool bataryIsHot = true;
bool pump1IsOn = true;
void setup()
{    
  Serial.begin(9600);  
  Serial.println("Setup>"); 
  initializeEthernet();
  initializeMQTT();
  pinMode(bataryController, OUTPUT);
  pinMode(pump1, OUTPUT);
  initializeThermometers();
  //update initial states
  switchBataryToHot(true);
  switchPump1ToOn(true);
  Serial.println("Setup<");  
  delay(500);//Wait for newly restarted system to stabilize
}

DeviceAddress *sensorsUnique;
// количество датчиков на шине
int countSensors;

void initializeThermometers(){
  sensors.begin();
  countSensors = 5;
  
  sensorsUnique = new DeviceAddress[5]{
    //28ab9707b6013c6e
    {0x28, 0xAB, 0x97, 0x07, 0xB6, 0x01, 0x3C, 0x6E},
    //28761007b6013c7a    
    {0x28, 0x76, 0x310, 0x07, 0xB6, 0x01, 0x3C, 0x7A},
    //2893c707b6013cb7
    {0x28, 0x93, 0xC7, 0x07, 0xB6, 0x01, 0x3C, 0xB7},
    //280b0c79a20003a7
    {0x28, 0x0B, 0x0C, 0x79, 0xA2, 0x00, 0x03, 0xA7},
    //282ccb07b6013cb5
    {0x28, 0x2C, 0xCB, 0x07, 0xB6, 0x01, 0x3C, 0xB5}
    };
  
  String allAddress = " ";
  for (int i = 0; i < countSensors; i++) {
    allAddress += " " + convertAddressToString(sensorsUnique[i]);
  }
  sendMQTTMessage("/heat/thermometersAddress", allAddress);
}

void initializeMQTT(){
  Serial.println("MQTT>");
  client.setServer(server, 8083 );
  client.setCallback(callback);
  
  if (client.connect(monitorConnectedTopic)) {
    sendMQTTMessage(monitorConnectedTopic, "OK");    
    Serial.println("MQTT+");
  }
  else{
    Serial.println("MQTT-");
  }
  client.subscribe("/heat/batary1/turn");
  Serial.println("MQTT<");
}

void initializeEthernet(){
  Serial.println("Ethernet>");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected");
    }    
  }else{
    Serial.print("IP address: ");
    Serial.println(Ethernet.localIP());  
  }  
  Serial.println("Ethernet<");
}

void callback(char* topic, byte* payload, unsigned int length) {
  /*Example of receiving commands from MQTT*/
  Serial.println("Received");
  payload[length] = '\0';
  Serial.print(topic);
  Serial.print("  ");
  String strTopic = String(topic);
  String strPayload = String((char*)payload);
  Serial.println(strPayload);
  if (strTopic == "/heat/batary1/turn") {
    if (strPayload == "1")     switchBataryToHot(true);
    else if (strPayload == "0") switchBataryToHot(false);
  }  
  if (strTopic == "/heat/pump1/turn")  {
    if (strPayload == "1")     switchPump1ToOn(true);
    else if (strPayload == "0") switchPump1ToOn(false);
  }
}

void loop(void)
{
  checkConnections();
  client.loop();
  checkThermometers();    
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
  sendMQTTMessage("/alive/arduino/", "OK");
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

void switchPump1ToOn(bool setOn){
  if (setOn && !pump1IsOn){
    digitalWrite(pump1, HIGH);
    pump1IsOn = true;
    sendMQTTMessage(pump1ControllerTopic, String(pump1IsOn));
  }
  if (!setOn && pump1IsOn){
    digitalWrite(pump1, LOW);
    pump1IsOn = false;
    sendMQTTMessage(pump1ControllerTopic, String(pump1IsOn));
  }
}

void checkThermometers(){
  sensors.requestTemperatures(); 
  for (int i = 0;  i < countSensors;  i++) {    
    String address = convertAddressToString(sensorsUnique[i]);
    float currentThermometerValue = sensors.getTempC(sensorsUnique[i]);
    if (address == "28ab9707b6013c6e") {
      roomThermometerValue = currentThermometerValue;
      sendMQTTMessage("/heat/termometer1", String(currentThermometerValue));          
    }
    sendMQTTMessage("/heat/" + address, String(currentThermometerValue));    
  }  
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