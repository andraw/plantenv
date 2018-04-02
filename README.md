# plantenv

ESP8266 based Soil Moisture, Temperature &amp; Humidity environment monitoring

## Getting Started

This document assumes you have a working installation of Arduino and a reasonable understanding of how Arduino and NodeMCU devices work.

The code was developed on a LoLin NodeMCU V3 development board.

### Prerequisites

You will need to install the following libraries into your Arduino installation:

* [pubsubclient](http://osoyoo.com/wp-content/uploads/samplecode/pubsubclient.zip) - Arduino MQTT Library
* [adafruit_sensor](https://github.com/adafruit/Adafruit_Sensor) - Adafruit Sensor Library
* [dht_sensor_library](https://github.com/adafruit/DHT-sensor-library) - Adafruit DHT Humidity & Temperature Unified Sensor Library

You will also need access to a MQTT server, which is beyond the scope of this README.

## Setting Up

### Setting up the hardware

```
DHT11 - Grid facing you
-----------------------

Pin 1 - Connect to a 3v pin on the NodeMCU
Pin 2 - Connect to D1 on the NodeMCU
Pin 3 - No connection
Pin 4 - Connect to ground on the NodeMCU


Soil Moisture Sensor
--------------------

A0  - Connect to A0 on the NodeMCU
GND - Connect to ground on the NodeMCU
VCC - Connect to a 3v pin on the NodeMCU


NodeMCU
-------

Connect RST to D0 if you wish to use LOW_POWER mode
```

### Setting up the software

Open plantenv.ino in the Arduino editor, you will need to make some changes to reflect your environment:

Variable | Description
-------- | -----------
LOW_POWER | Valid values are 0 and 1.  If LOW_POWER is set to 1 then the device will run in a low power mode, if it is left at the default of 0 then the webserver will be enabled.
USE_DHCP | Valid values are 0 and 1.  If USE_DHCP is set to 1 then the device will attempt to aquire it's IP address from the network.  Setting USE_DHCP to 0 allows you to specify the IP address and gateway for the device.
ssid | Set this string to match the wireless network you wish the device to connect to.
password | Set this string to match the password required to connect to the wireless network.
mqtt_topic_start | This is the topic that plantenv will publish to on startup (or when going to sleep in LOW_POWER mode).
mqtt_topic_temp | This is the topic that plantenv will publish the current temperature on.
mqtt_topic_rh | This is the topic that plantenv will publish the current relative humidity on.
mqtt_topic_moisture | This is the topic that plantenv will publish the current soil moisture on.
mqtt_topic_wifi_signal | This is the topic that plantenv will publish the current wifi signal strength on.
mqtt_server | The IP address of the MQTT server.  
mqtt_client_id | The identification used for the MQTT server.
low_sensor_value | This will probably be ok.  If you have an problems then set this to be the value returned from the soil sensor with no sensor connected.  You will have to put a print statement into the code and watch the serial console output.
publish_time | When not running in LOW_POWER mode this is how often plantenv will publish (in seconds).
deep_sleep | When running in LOW_POWER mode this is the interval to sleep for (in seconds).
soil_sensor_pin | If you follow the wiring diagram then you should not need to change this.
DHTTYPE | This is the type of DHT sensor you have connected - it'll probably be DHT11.
DHTPin | If you follow the wiring diagram then you should not need to change this.
WiFiServer server(80) | You can change from the default http port (80) by altering the value in server.

When you are happy with your configuration, upload it to the NodeMCU board.  Your settings may vary, but in my case the board type was set to "Node MCU 1.0", CPU Frequency at "80 MHz" and an upload speed of "115200".  When it is uploaded, connect to the Serial Monitor, and you should see something like:

```
[plantenv] Starting ...
[plantenv] Connecting to home-iot with STATIC IP: ..............
[plantenv] WiFi Connected.
[plantenv] Connected to home-iot
[plantenv] IP address: 192.168.1.220
[plantenv] MAC address: BC:DD:C2:FE:D9:04
[plantenv] WiFi Strength: -78
[plantenv] Starting DHT Sensor [plantenv] Starting HTTP Server
[plantenv] HTTP Server URL: http://192.168.1.220/
[plantenv] Setup Complete - Starting Main Loop
[plantenv] Attempting MQTT connection...connected
[tele/plantenv/BCDDC2FED904/wifi_signal] -79
[tele/plantenv/BCDDC2FED904/moisture] 0
[tele/plantenv/BCDDC2FED904/temp] 25.00
[tele/plantenv/BCDDC2FED904/rh] 36.00
```

You should now see MQTT transmissions from the device, if your not in LOW_POWER mode then you should also be able to connect to the HTTP interface.

## Authors

* **Andrew Rawlins** - *Initial work* - [www.fermit.org.uk](http://www.fermit.org.uk) 
