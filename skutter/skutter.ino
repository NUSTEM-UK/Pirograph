#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Configure appropriately to your local network
const char* ssid = "nustem";
const char* password = "nustem123";
const char* mqtt_server = "10.0.1.3";

// wifi networking and connection to MQTT broker
WiFiClient espClient;
PubSubClient client(espClient);

// Each Skutter announces itself with its hardware MAC address
String skutterMACaddress;

// Let's have a buffer to hold the JSON payload of incoming messages
const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
DynamicJsonBuffer jsonBuffer(capacity);

void setup() {
  // We're going to spew debug strings to Serial, because of course we are
  Serial.begin(115200);

  // Fire up the radio!
  setup_wifi();

  // Get MAC address
  skutterMACaddress = WiFi.macAddress();

  // Connect to MQTT broker, register callback on message receipt
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  // Call the MQTT client to poll for updates, reconnecting if necessary
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // TODO: call an animation update function here.
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println(">>> MQTT message received");
  Serial.print("topic: ");
  Serial.println(topic);
  Serial.print("paylaod: ");
  Serial.println(*payload);

  // Parse payload as JSON, into root object
  JsonObject& root = jsonBuffer.parseObject(payload);
  if (!root.success()) {
    Serial.println(">>> Oh noes! JSON parsing failed!");
    // Just bail out and hope the next message makes sense
    return;
  }

  // Check if we're the skutter that's being spoken to.
  // TODO: Parse a tuple here: there may be >1 skutter being addressed at once.
  if (root["name"] == skutterMACaddress) {
    Serial.println(">>> DING DING! I'm being instructed!");

    // Now we start handling the individual command sequences. Which is the gnarly bit.
    // Or at least, _a_ gnarly bit.
  }
}

