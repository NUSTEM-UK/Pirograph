/* Reset buttons to command the Processing sketch without walking to the keyboard

*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "nustem";
const char* password = "nustem123";
const char* mqtt_server = "10.0.1.3";

WiFiClient espClient;
PubSubClient client(espClient);
// Each robot has a unique name, generated from the hardware MAC address.
// These variables will store those names.
char skutterNameArray[60];
//String huzzahMACAddress;
String skutterNameString;
String subsTargetString;
char subsTargetArray[60];

long lastMsg = 0;
char msg[50];
int value = 0;

#define PIN_BUTTON_RESET D1
#define PIN_BUTTON_SAVE D2
#define PIN_LED_BLUE D4
#define PIN_LED_RED D3


void setup() {
    Serial.begin(115200);
    setup_wifi();

    client.setServer(mqtt_server, 1883);
    // client.setCallback(callback); // We don't really care, frankly.

    pinMode(PIN_LED_BLUE, OUTPUT);
    pinMode(PIN_LED_RED, OUTPUT);

    pinMode(PIN_BUTTON_RESET, INPUT_PULLUP);
    pinMode(PIN_BUTTON_SAVE, INPUT_PULLUP);

}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    if (!digitalRead(PIN_BUTTON_RESET)) {
        Serial.println("Sending RESET");
        client.publish("reset", "1");
    }
    if (!digitalRead(PIN_BUTTON_SAVE)) {
        Serial.println("Sending SAVE");
        client.publish("save", "1");
    }

}
