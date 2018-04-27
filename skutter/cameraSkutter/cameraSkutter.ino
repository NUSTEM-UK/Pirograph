#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// here comes the hot-steppers
#include <AccelStepper.h>
#define HALFSTEP 8

// Motor pin definitions - Stepper Speed 
#define motorPin1  D5     // IN1 on the ULN2003 driver 1
#define motorPin2  D6     // IN2 on the ULN2003 driver 1
#define motorPin3  D7     // IN3 on the ULN2003 driver 1
#define motorPin4  D8     // IN4 on the ULN2003 driver 1

AccelStepper pitchStepper(HALFSTEP, motorPin1, motorPin3, motorPin2, motorPin4);

// Motor pin definitions - Stepper Speed 
#define motorPin5  D1     // IN1 on the ULN2003 driver 1
#define motorPin6  D2     // IN2 on the ULN2003 driver 1
#define motorPin7  D3     // IN3 on the ULN2003 driver 1
#define motorPin8  D4     // IN4 on the ULN2003 driver 1

AccelStepper rollStepper(HALFSTEP, motorPin5, motorPin7, motorPin6, motorPin8);

int tenSteps = 100;

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

void setup() {
    Serial.begin(115200);
    setup_wifi();

    // Get this Wemos' MAC address and use it to register with the MQTT server
    skutterNameString = WiFi.macAddress();
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

    rollStepper.setMaxSpeed(1000);
    rollStepper.setAcceleration(1000);
    pitchStepper.setMaxSpeed(1000);
    pitchStepper.setAcceleration(1000);
}

void loop() {
    // Call the MQTT client to poll for updates, reconnecting if necessary
    // Handle messages via the callback function
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
    pitchStepper.run();
    rollStepper.run();
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.println(">>> MQTT message received");
    String payloadString;
    for (int i = 0; i < length ; i++) {
        payloadString += String((char)payload[i]);
    }
    Serial.print("payload (string): ");
    Serial.println(payloadString);

    StaticJsonBuffer<256> jsonBuffer;

    JsonObject& root = jsonBuffer.parseObject(payload);
    if (!root.success()) {
        Serial.println("Parsing failed!");
        return;
    }    
    Serial.print("Command: ");
    Serial.println(root["command"].as<String>());

    if (root["command"] == "setPitch") {
        int target = root["value"];
        pitchStepper.move(target);
        }
    
    if (root["command"] == "setRoll") {
        int target = root["value"];
        rollStepper.move(target);
        }
    
}