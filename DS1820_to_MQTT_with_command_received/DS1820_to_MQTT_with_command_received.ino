#include <SPI.h> //Ethernet shield
#include <Ethernet.h> //Ethernet shield

#include <PubSubClient.h> //MQTT

#include <OneWire.h> //Temperature
#include <DallasTemperature.h> //Temperature

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

char* monitorConnectedTopic = "/heat/temperatureMonitor";
String thermometersCountTopic = "/heat/thermometersCount";
String thermometersAddressTopic = "/heat/thermometersAddress";
String thermometerTmpTopic = "/heat/termometer1";
String bataryControllerTopic = "/heat/batary1";
char* bataryActionTopic = "/heat/batary1/turn";
String thermometersInfoTopicStart = "/heat/";
String arduinoAliveTopic = "/alive/arduino/";

bool bataryIsHot = true;
void setup()
{    
  Serial.begin(9600);  
  Serial.println("Setup started");
  sensors.begin();      
  initializeEthernet();
  initializeMQTT();
  pinMode(bataryController, OUTPUT);
  Serial.println("Setup finished");  
  delay(500);//Wait for newly restarted system to stabilize
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
  sendMQTTMessage(arduinoAliveTopic, "OK");
  client.loop();
  checkThermometers();    
  delay(60000);
}

void sendMQTTMessage(String topic, String message){
  Serial.println(topic + ": " + message);  
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
  sensors.requestTemperatures();
  float temp=sensors.getTempCByIndex(0);   
  sendMQTTMessage(thermometerTmpTopic, String(temp));
  if (temp < 18){
    switchBataryToHot(true);
  }
  if (temp > 26){
    switchBataryToHot(false);
  }
  
}