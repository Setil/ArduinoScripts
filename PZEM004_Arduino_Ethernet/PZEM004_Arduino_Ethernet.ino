#include <SPI.h> //Ethernet shield
#include <Ethernet.h> //Ethernet shield
#include <PubSubClient.h> //MQTT
#include <Arduino.h>
#include <PZEM004Tv30.h> //PZEM
#include <SoftwareSerial.h>

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
byte server[] = { 192, 168, 1, 180 };

EthernetClient ethClient;
PubSubClient client(ethClient);
String electroVoltage = "/electro/voltage1";
String electroCurrent = "/electro/current1";
String electroPower = "/electro/power1";
String electroEnergy = "/electro/energy1";
String electroFrequency = "/electro/frequency1";
String electroPowerfactor = "/electro/powerfactor1";
char* arduinoAliveTopic = "/alive/arduino/";
SoftwareSerial pzemSWSerial(9, 8);
PZEM004Tv30 pzem;

void setup()
{    
  Serial.begin(9600);  
  Serial.println("Setup started");
  initializeEthernet();
  initializeMQTT();
  Serial.println("Setup finished");  
  pzem = PZEM004Tv30(pzemSWSerial);
  delay(500);//Wait for newly restarted system to stabilize
}

void initializeMQTT(){
  Serial.println("MQTT initialize started");
  client.setServer(server, 8083 );
  client.setCallback(callback);
  
  if (client.connect(arduinoAliveTopic)) {
    sendMQTTMessage(arduinoAliveTopic, "OK");    
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
}

void loop(void)
{
  checkConnections();
  client.loop();
  
    // Read the data from the sensor
  float voltage = pzem.voltage();
  float current = pzem.current();
  float power = pzem.power();
  float energy = pzem.energy();
  float frequency = pzem.frequency();
  float pf = pzem.pf();


    // Check if the data is valid
  if(isnan(voltage)){
      Serial.println("Error reading voltage");
      sendMQTTMessage(electroVoltage, "-100");
  } else if (isnan(current)) {
      Serial.println("Error reading current");
      sendMQTTMessage(electroCurrent, "-100");      
  } else if (isnan(power)) {
      Serial.println("Error reading power");
      sendMQTTMessage(electroPower, "-100");
  } else if (isnan(energy)) {
      Serial.println("Error reading energy");
      sendMQTTMessage(electroEnergy, "-100");
  } else if (isnan(frequency)) {
      Serial.println("Error reading frequency");
      sendMQTTMessage(electroFrequency, "-100");
  } else if (isnan(pf)) {
      Serial.println("Error reading power factor");
      sendMQTTMessage(electroPowerfactor, "-100");
  } else {
    // Print the values to the Serial console
    Serial.print("Voltage: ");      Serial.print(voltage);      Serial.println("V");
    Serial.print("Current: ");      Serial.print(current);      Serial.println("A");
    Serial.print("Power: ");        Serial.print(power);        Serial.println("W");
    Serial.print("Energy: ");       Serial.print(energy,3);     Serial.println("kWh");
    Serial.print("Frequency: ");    Serial.print(frequency, 1); Serial.println("Hz");
    Serial.print("PF: ");           Serial.println(pf);
    sendMQTTMessage(electroVoltage, String(voltage));
    sendMQTTMessage(electroCurrent, String(current));
    sendMQTTMessage(electroPower, String(power));
    sendMQTTMessage(electroEnergy, String(energy));
    sendMQTTMessage(electroFrequency, String(frequency));
    sendMQTTMessage(electroPowerfactor, String(pf));
  }

  Serial.println();
  delay(5000);
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
