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
SoftwareSerial pzemSWSerial(9, 8);
PZEM004Tv30 pzem;
char* arduinoAliveTopic = "/alive/arduino/";
void setup()
{    
  Serial.begin(9600);  
  Serial.println("Setup>");
  initializeEthernet();
  initializeMQTT();
  pzem = PZEM004Tv30(pzemSWSerial);
  Serial.println("Setup<");  
  
  delay(500);//Wait for newly restarted system to stabilize
}

void initializeMQTT(){
  Serial.println("MQTT>");
  client.setServer(server, 8083 );
  
  if (client.connect(arduinoAliveTopic)) {
    sendMQTTMessage(arduinoAliveTopic, "OK");    
    Serial.println("MQTT ok");
  }
  else{
    Serial.println("MQTT not ok");
  }  
  Serial.println("MQTT<");
}

void initializeEthernet(){
  Serial.println("Ethernet>");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("No ip");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("No ethernet shield");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("No cable");
    }    
  }else{
    Serial.print("My IP: ");
    Serial.println(Ethernet.localIP());  
  }  
  Serial.println("Ethernet<");
}

void loop(void)
{
  checkConnections();  
  
    // Read the data from the sensor
  float voltage = pzem.voltage();
  float current = pzem.current();
  float power = pzem.power();
  float energy = pzem.energy();
  float frequency = pzem.frequency();
  float pf = pzem.pf();
  String electroVoltage = "/electro/voltage1";
  String electroCurrent = "/electro/current1";
  String electroPower = "/electro/power1";
  String electroEnergy = "/electro/energy1";
  String electroFrequency = "/electro/frequency1";
  String electroPowerfactor = "/electro/powerfactor1";


    // Check if the data is valid
  if(isnan(voltage)){
      Serial.println("Error voltage");
      sendMQTTMessage(electroVoltage, "-100");
  } else if (isnan(current)) {
      Serial.println("Error current");
      sendMQTTMessage(electroCurrent, "-100");      
  } else if (isnan(power)) {
      Serial.println("Error power");
      sendMQTTMessage(electroPower, "-100");
  } else if (isnan(energy)) {
      Serial.println("Error energy");
      sendMQTTMessage(electroEnergy, "-100");
  } else if (isnan(frequency)) {
      Serial.println("Error frequency");
      sendMQTTMessage(electroFrequency, "-100");
  } else if (isnan(pf)) {
      Serial.println("Error power factor");
      sendMQTTMessage(electroPowerfactor, "-100");
  } else {
    // Print the values to the Serial console
    /*
    Serial.print("Voltage: ");      Serial.print(voltage);      Serial.println("V");
    Serial.print("Current: ");      Serial.print(current);      Serial.println("A");
    Serial.print("Power: ");        Serial.print(power);        Serial.println("W");
    Serial.print("Energy: ");       Serial.print(energy,3);     Serial.println("kWh");
    Serial.print("Frequency: ");    Serial.print(frequency, 1); Serial.println("Hz");
    Serial.print("PF: ");           Serial.println(pf);*/
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
