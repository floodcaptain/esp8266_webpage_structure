# esp8266_webpage_structure

## Brief
this sketch is a reference to host a webpage in AP mode.
when wifi credentials are keyed in, esp restarts and attempts to connect to keyed in 
wifi parameters.

## Dependencies

ESP8266HTTPClient.h
ESP8266WiFi.h
ESP8266WebServer.h
EEPROM.h

## Reference Links

https://github.com/dreamsparkx/ESP8266-WiFi-Config

wifi:
https://github.com/esp8266/Arduino/blob/master/doc/esp8266wifi/scan-class.rst
https://github.com/esp8266/Arduino/issues/529
http://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/soft-access-point-class.html#set-up-network


eeprom:
http://esp8266.github.io/Arduino/versions/2.0.0/doc/libraries.html
https://circuits4you.com/2016/12/16/esp8266-internal-eeprom-arduino/

http:
https://circuits4you.com/2016/12/16/esp8266-internal-eeprom-arduino/
https://circuits4you.com/2016/12/16/esp8266-web-server/

example codes:
http://www.microcontroller-project.com/esp8266-post-data-to-thingspeak-server.html