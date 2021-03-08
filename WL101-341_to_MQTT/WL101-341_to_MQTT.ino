#include <RCSwitch.h>

#include <SPI.h> //Ethernet shield
#include <Ethernet.h> //Ethernet shield

#include <PubSubClient.h> //MQTT

byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};
byte server[] = { 192, 168, 1, 21 };
long coAlarmId = 112423;

EthernetClient ethClient;
PubSubClient client(ethClient);

RCSwitch mySwitch = RCSwitch();
String alarmTopic = "/gaz/co/alarm/";
char* alarmConnectedTopic = "/gaz/";
void setup() {
  Serial.begin(9600);
  Serial.println("SETUP START");
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
  initializeEthernet();
  initializeMQTT();
  Serial.println("SETUP FINISHED");
}

void loop() {    
  if (mySwitch.available()) {
    long received = mySwitch.getReceivedValue();
    if (received == coAlarmId){    
      sendMQTTMessage(alarmTopic + String(coAlarmId), "1");      
    }        
    mySwitch.resetAvailable();    
  }
  else{
    Serial.println("NO MESSAGE");
  }
  delay(1000);
}

void initializeMQTT(){
  Serial.println("MQTT initialize started");
  client.setServer(server, 1883);
  if (client.connect(alarmConnectedTopic)) {
    sendMQTTMessage(alarmConnectedTopic, "OK");    
    Serial.println("MQTT Working");    
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

void sendMQTTMessage(String topic, String message){
  Serial.println(topic + ": " + message);  
  client.publish(topic.c_str(),message.c_str());      
}
