#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "nustem";
const char* password = "nustem123";
const char* mqtt_server = "10.0.1.3";

WiFiClient espClient;
PubSubClient client(espClient);

// Each robot has a unique name, generated from the hardware MAC address.
// These variables will store those names.
char skutterNameArray[60];
String huzzahMACAddress;
String skutterNameString;
String subsTargetString;
char subsTargetArray[60];

const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
DynamicJsonBuffer jsonBuffer(capacity);

void setup() {
  Serial.begin(115200);
  setup_wifi();

  // Get this Huzzah's MAC address and use it to register with the MQTT server
  huzzahMACAddress = WiFi.macAddress();
  skutterNameString = "skutter_" + huzzahMACAddress;
  Serial.println(skutterNameString);
  skutterNameString.toCharArray(skutterNameArray, 60);
  subsTargetString = "pirograph/" + skutterNameString;
  subsTargetString.toCharArray(subsTargetArray, 60);
  for (int i = 0; i < 60; i++) {
      Serial.print(subsTargetArray[i]);
  }
  Serial.println();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

void loop() {
  // Call the MQTT client to poll for updates, reconnecting if necessary
  // Handle messages via the callback function
  if (!client.connected()) {
      reconnect();
  }
  client.loop();

}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println(">>> MQTT message received");
  Serial.print("topic: ");
  Serial.println(topic);
  Serial.print("payload: ");
  Serial.println(*payload);

  String payloadString;
  for (int i = 0; i < length ; i++) {
    payloadString += String((char)payload[i]);
  }
  Serial.print("payload (string): ");
  Serial.println(payloadString);

  JsonObject& root = jsonBuffer.parseObject(payload);
  if (!root.success()) {
    Serial.println("Parsing failed!");
    return;
  }

  Serial.print("Name: ");
  Serial.println(root["name"].as<char*>());

  // String handling: see https://arduinojson.org/example/string/
  // Can cast to String on parse:
  Serial.println(root["name"].as<String>());

  // Can even compare to a String:
  String testString = "Bob";
  if (root["name"] == testString) {
    Serial.println(">>> DING DING! We have a Bob!");
  }
  
}
