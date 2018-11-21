# WebUpdate
Minimal sketch running a webserver for firmware updates

## Required Hardware
Runs on an ESP8266 (e.g. a NodeMCU)

## Build Instructions
* Build your firmware with PlatformIO or Arduino IDE
* For PlatformIO you can edit platformio.ini to configure the build (hostname, wlan, ...)
* For Arduino IDE you need to edit the source file for that.
* You can define *ONLINE_PIN* to drive a led showing the online state
* TODO: You can define *CONTROL_PIN* to control the connection process

## WLAN
* You define *SSID* and *PASS* (in WlanConfig.h or build flags or ...)
* Also define *NAME* and *PORT* to be used as hostname:httpservice
* The device will use DHCP to get an IP address once connected to the WLAN
* The device will show up on your network as `http://$NAME.local`
* The device will run an internal web server using *PORT* (Default: 80)

## Usage
Once connected to the wlan you can update the firmware
with a browser or a web cli tool, e.g. with firmware file firmware.bin: 
```
curl -vF 'image=@firmware.bin' $NAME.local:$PORT/update
```
Have fun!
