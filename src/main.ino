#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Requires the following libraries:
// Adafruit Unified Sensors
// DHT Sensor Library
// PubSubClient
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 2
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

#include <PubSubClient.h>

// config.h should have the following contents:
/*
const char *ssid = "WIFI_SSID";
const char *password = "WIFI_PASSWORD";
const char *mqtt_server = "my.mqtt.server.com";
const char *mqtt_topic = "my_mqtt_topic";
*/

#include "config.h"

WiFiClient espClient;
PubSubClient client(espClient);

String dataString;
char charBuf[256];

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  // setup serial port
  Serial.begin(9600);

  // setup WiFi
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

float humidity, temp_f;

void getTemperature() {
  humidity = dht.readHumidity();
  temp_f = dht.readTemperature();
  Serial.println(humidity);
  Serial.println(temp_f);
}

void reconnect() {
  //WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    setup_wifi();
  }
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266_Client")) {
      Serial.println("connected");
    } else {
      Serial.print("MQTT connection failed, retry count: ");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  getTemperature();
  Serial.printf("Publishing to server: %s - topic: %s\n", mqtt_server, mqtt_topic);
  dataString = String("sensors,location=livingroom temperature=") + temp_f + String(",humidity=") + humidity;
  dataString.toCharArray(charBuf, 250);
  Serial.println(charBuf);
  client.publish(mqtt_topic, charBuf);
  delay(5000);
  Serial.println( "Closing MQTT connection...");
  client.disconnect();
  Serial.println( "Closing WiFi connection...");
  WiFi.disconnect();
  Serial.println( "Sleeping for a minute");
  delay(60000);
}
