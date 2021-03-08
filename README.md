# ArduinoMQTTTemperature
Arduion scketch for sending multiple thermometers info to MQTT

Script sends info in topics:

/heat/temperatureMonitor - OK message when connected

/heat/thermometersCount - how many thermometers are connected to arduino plate

/heat/thermometersAddress - mac-addresses of all thermometers separated by space

/heat/<mac-address> - temperature in C of concreete thremometer
