/* plantenv - ESP8266 Soil Moisture, Temperature & Humidity

   Reads Soil Moisture, Temperature & Humidity and
   transmits the same via MQTT.  Optionally a local
   webserver can be enabled to view the data.

   v1.0 - (c) 2018, Andrew Rawlins

   You'll need the following libraries:

   http://osoyoo.com/wp-content/uploads/samplecode/pubsubclient.zip
   https://github.com/adafruit/Adafruit_Sensor
   https://github.com/adafruit/DHT-sensor-library

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// ------------------------------------------
// Change for your local environment
// ------------------------------------------

// If LOW_POWER is set to 0 then we will run a webserver
// and allow constant monitoring.  If LOW_POWER is set to
// 1 then we'll only publish our data via MQTT every 15
// minutes or so, and go into a deep sleep inbetween.
#define LOW_POWER 0

// If USE_DHCP is set to 1 then we will use DHCP to
// configure our wireless IP otherwise we will use a static
// IP.  If a static IP is needed you will need to set it
// further down in the code.
#define USE_DHCP 0

// Your Wireless SSD & Password
const char* ssid = "home-iot";
const char* password = "givemewifi";

// Your MQTT Server
const char* mqtt_server = "192.168.1.200";
const char* mqtt_client_id = "MY_DEVICE_ID";

// We should probably do this better
const char* mqtt_topic_start = "tele/plantenv/status";
const char* mqtt_topic_temp = "tele/plantenv/temp";
const char* mqtt_topic_rh = "tele/plantenv/rh";
const char* mqtt_topic_moisture = "tele/plantenv/moisture";
const char* mqtt_topic_wifi_signal = "tele/plantenv/wifi_signal";

// Configure this depending on the zero reading
// from your soil sensor (i.e not connected).
int low_sensor_value = 415;

// How many seconds should we publish when not in LOW_POWER
const int publish_time = 300;

// How long should we go into a deep sleep for this is the
// wake up interval in seconds.  Requires LOW_POWER to be
// set to 1
const int deep_sleep = 3600;

// Soil Sensor input at Analog PIN A0
int soil_sensor_pin = A0;
int soil_sensor_value;

// DHT Sensor on D6
#define DHTTYPE DHT11
const int DHTPin = 5;
DHT dht (DHTPin, DHTTYPE);

// Internal clock
int publoop = 0;

int WiFiStrength = 0;
// Web Server (if we are not LOW_POWER), 80 is the port
WiFiServer server(80);

// MQTT
WiFiClient espClient;
PubSubClient client(espClient);


// ------------------------------------------
// Look after the MQTT Connection
// ------------------------------------------
void reconnect() {

  // Loop until we're reconnected

  while (!client.connected()) {
    Serial.print("[plantenv] Attempting MQTT connection...");

    // Attempt to connect

    if (client.connect(mqtt_client_id)) {

      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(mqtt_topic_start, "ONLINE");

    } else {

      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);

    }
  }
}


void setup() {

  Serial.begin(115200);
  delay(10);

  Serial.println("[plantenv] Starting ...");
  Serial.print("[plantenv] Connecting to ");
  Serial.print(ssid);

#if USE_DHCP == 1
  Serial.print(" via DHCP: ");
  WiFi.begin(ssid, password);
#else
  WiFi.begin(ssid, password);
  Serial.print(" with STATIC IP: ");
  // Replace the values below if you wish to use a
  // static IP address
  WiFi.config(IPAddress(192, 168, 1, 220), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
#endif

  // Wait for a network connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  Serial.println("[plantenv] WiFi Connected.");
  Serial.print("[plantenv] Connected to ");
  Serial.println(ssid);
  Serial.print("[plantenv] IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("[plantenv] MAC address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("[plantenv] WiFi Strength: ");
  Serial.println(WiFi.RSSI());

  client.setServer(mqtt_server, 1883);
  client.publish(mqtt_topic_start, "ONLINE");

  Serial.print("[plantenv] Starting DHT Sensor ");
  dht.begin();

#if LOW_POWER == 0
  Serial.println("[plantenv] Starting HTTP Server");
  // Start the web server
  server.begin();
  Serial.print("[plantenv] HTTP Server URL: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
#else
  Serial.println("[plantenv] Starting in LOW POWER mode.");
#endif

  Serial.println("[plantenv] Setup Complete - Starting Main Loop");

  delay(2000);

}

void loop() {

  // We publish every n seconds
  for (int i = 0; i <= (publish_time + 1); i++) {

    // Make sure we can talk to the MQTT Server
    if (!client.connected()) {
      reconnect();
    }

    // ------------------------------------------
    // Get all of our values
    // ------------------------------------------
    WiFiStrength = WiFi.RSSI();
    soil_sensor_value = analogRead(soil_sensor_pin);
    soil_sensor_value = map(soil_sensor_value, low_sensor_value, 0, 0, 100);

    float rh_sensor_value = dht.readHumidity();
    float temp_sensor_value = dht.readTemperature();

    if (isnan(rh_sensor_value) || isnan(temp_sensor_value)) {

      // We've failed to read :(
      Serial.println("[plantenv] Failed to read DHT sensor");
      client.publish(mqtt_topic_start, "ERROR_IN_DHT_READ");

      rh_sensor_value = 0;
      temp_sensor_value = 0;

    }

    // Time to Publish
    if (publoop == publish_time) {
      publoop = 0;
    }

#if LOW_POWER == 1
    // Then it's always time to publish, we are not awake for long
    publoop = 0;
#endif

    // We publish at the start of the loop and then wait
    if (publoop == 0) {
      Serial.print("[");
      Serial.print(mqtt_topic_wifi_signal);
      Serial.print("] ");
      Serial.println(WiFiStrength);

      char wifi_strength_value_buf [4];
      sprintf (wifi_strength_value_buf, "%03i", WiFiStrength);
      client.publish(mqtt_topic_wifi_signal, wifi_strength_value_buf);

      Serial.print("[");
      Serial.print(mqtt_topic_moisture);
      Serial.print("] ");
      Serial.println(soil_sensor_value);

      char soil_sensor_value_buf [3];
      sprintf (soil_sensor_value_buf, "%02i", soil_sensor_value);
      client.publish(mqtt_topic_moisture, soil_sensor_value_buf);

      Serial.print("[");
      Serial.print(mqtt_topic_temp);
      Serial.print("] ");
      Serial.println(temp_sensor_value);

      char temp_sensor_value_buf [2];
      sprintf (temp_sensor_value_buf, "%d.%02d", (int)temp_sensor_value, (int)(temp_sensor_value * 100) % 100);
      client.publish(mqtt_topic_temp, temp_sensor_value_buf);

      Serial.print("[");
      Serial.print(mqtt_topic_rh);
      Serial.print("] ");
      Serial.println(rh_sensor_value);
      char rh_sensor_value_buf [2];
      sprintf (rh_sensor_value_buf, "%d.%02d", (int)rh_sensor_value, (int)(rh_sensor_value * 100) % 100);
      client.publish(mqtt_topic_rh, rh_sensor_value_buf);

      // Carry on the counter
      publoop = 1;

    }

    // Go into a Deep sleep if we are in low power mode
#if LOW_POWER == 1
    delay(500);
    Serial.println("[plantenv] Sleeping, night night");
    client.publish(mqtt_topic_start, "OFFLINE");

    // Need enough time to send our last message
    delay(1000);

    // And off to sleep we go
    ESP.deepSleep(deep_sleep * 1000000);
#endif

    // Carry on our counter
    delay(1000);
    publoop++;

    // We run a webserver when not in LOW_POWER so service
    // any requests
#if LOW_POWER == 0
    // Check for anything pending
    WiFiClient client = server.available();

    if (client) {
      
      // And if we have something then send it

      Serial.print("[plantenv] HTTP client Request: ");
      // Read the first line of the request
      String request = client.readStringUntil('\r');
      Serial.println(request);
      client.flush();

      // Return the response
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println(""); //  do not forget this one
      client.println("<!DOCTYPE HTML>");

      client.println("<html>");
      client.println("  <head>");
      client.println("    <title>ESP8266 Soil Moisture, Temperature & Humidity Sensor</title>");
      client.println("    <meta http-equiv=\"refresh\" content=\"60\">");
      client.println("  </head>");
      client.println("  <body>");
      client.println("  <table width=100%>");
      client.println("    <tr><td bgcolor=#000000>");
      client.print("    <h1 style=\"size:12px;font-family:arial,helvetica,sans-serif;color:white;\">");
      client.print("&nbsp;ESP8266 Soil Moisture, Temperature & Humidity Sensor - ");
      client.print(WiFi.macAddress());
      client.println("<h1>");
      client.println("    </td></tr>");
      client.println("  </table>");

      client.println("<hr>");

      client.print("<b>Soil Moisture:</b> ");
      client.print(soil_sensor_value);
      client.println("&#37;<br>");

      client.print("<b>Temperature:</b> ");
      client.print(temp_sensor_value);
      client.println("&deg;C<br>");

      client.print("<b>RH: </b>");
      client.print(rh_sensor_value);
      client.println("&#37;<br><br><br>");

      client.print("<b>WiFi Signal Strength: </b>");
      client.print(WiFiStrength);
      client.println("dBm<br>");

      client.print("<b>Local IP: </b>");
      client.print(WiFi.localIP());
      client.println("<br>");
      
      client.print("<b>Hardware (MAC) Address: </b>");
      client.print(WiFi.macAddress());
      client.println("<br>");
      
      client.println("<hr>");
      client.print("plantenv (c)2018, Andrew Rawlins - ");
      client.print("<a href = \"https://github.com/andraw/plantenv\">https://github.com/andraw/plantenv</a>");
      client.println("</body>");
      client.println("</html>");
    }
    
#endif

  }

}



