# DS1820MultipleTemperature_to_MQTT

Arduino scketch for sending multiple thermometers info to MQTT

Script sends info in topics:

/heat/temperatureMonitor - OK message when connected

/heat/thermometersCount - how many thermometers are connected to arduino plate

/heat/thermometersAddress - mac-addresses of all thermometers separated by space

/heat/<mac-address> - temperature in C of concreete thremometer

#  WL101-341_to_MQTT
Arduino scketch for sending alarm message from carbon monoxide alarm sensor via wireless 433MHz received by WL101-341 to MQTT .

COAlarm only sends it's ID when alarm is triggered (by real CO (smoke) or by pressing test button). 

COAlarm send only one message when alaram is triggered (when CO ppt becomes greater 100). No additional messages when CO ppt continue to increase.  No message when disalarm (smoke is gone (CO ppt less than 100) or shutdown by long press test button). 

Alarm ID is hardcoded and checked to prevent fake alarms from other wireless items

# MHZ19C_UART
Arduino scketch for reading CO2 PPM data from MHZ19C sensor
