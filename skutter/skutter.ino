/* "Skutters" -- Wemos D1 mini-based robots which control servos/steppers,
along with NeoPixel RGB LED strips. Responding to messages passed over MQTT,
they trace the light patterns picked up by the camera and image processing
components.

Based on WishingWell_Skutter from the Tech-Wishing-Well project.
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Servo.h>
#include <FastLED.h> // Using FastLED not NeoPixel, to gain HSV colour support

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

// Overkill of a static buffer, but we're an ESP8266, we have bytes to burn.
StaticJsonBuffer<1024> jsonBuffer;

// Servo and LED strip setup
#define PIN_SERVO1 D7
#define PIN_SERVO2 D6
#define PIN_LED_BLUE D4
#define PIN_LED_RED D3
#define PIN_PIXEL D1

#define PIXEL_COUNT 1

Servo servo1;
Servo servo2;

float servo1position = 90;
float servo1target = 90;
float servo2position = 90;
float servo2target = 90;

// Array of LEDs - current values
CRGB leds[PIXEL_COUNT];
// Need shadow array of HSV values, as the pixels are RGB objects but there's no
// conversion back to HSV.
// see: https://github.com/FastLED/FastLED/wiki/Pixel-reference
CHSV ledsHSV[PIXEL_COUNT];

// LEDs - target values
CRGB ledsTarget[PIXEL_COUNT];
CHSV ledsHSVTarget[PIXEL_COUNT];

uint8_t in_transition = 0; // 1: forwards; 0: no transition; -1: backwards
uint32_t time_start = millis();     // Start time of commanded transition
uint32_t time_end = millis();       // End time of commanded transition
uimt32_t time_current = millis();   // Recalculated in transition loop
uint32_t transition_time = 5;
String transitionType = "";

void setup() {
    Serial.begin(115200);
    setup_wifi();

    // Get this Huzzah's MAC address and use it to register with the MQTT server
    //  huzzahMACAddress = WiFi.macAddress();
    //  skutterNameString = "skutter_" + huzzahMACAddress;
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

    FastLED.addLeds<WS2811, PIN_PIXEL, GRB>(leds, PIXEL_COUNT);
    for (int targetLED = 0; targetLED < PIXEL_COUNT; targetLED += 1) {
        CHSV tempColour = CHSV(0, 255, 0);
        leds[targetLED] = tempColour;
        ledsHSV[targetLED] = tempColour;
    }
    FastLED.show();

    // Enable the built-in LEDs for network diagnostics
    pinMode(PIN_LED_BLUE, OUTPUT);
    pinMode(PIN_LED_RED, OUTPUT);

    servo1.attach(PIN_SERVO1);
    servo1.write(servo1position);
    servo2.attach(PIN_SERVO2);
    servo2.write(servo2position);

}

void loop() {
    // Call the MQTT client to poll for updates, reconnecting if necessary
    // Handle messages via the callback function
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    if (in_transition != 0) {
        time_current = millis(); // When is now?
        // Should we have already completed the current transition?
        if (time_current < time_end) {
            // Loop over the LED strip and update colours
            for (int i = 0; i < PIXEL_COUNT; i++) {
                
            }
        }
    }

    FastLED.show();

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

    // TODO: Error check the parsed json for a key before we use it.

    Serial.print("Command: ");
    Serial.println(root["command"].as<String>());
    // Serial.println(root["command"].as<char*>());
    // Serial.println(root["value"].as<char*>());

    // String handling: see https://arduinojson.org/example/string/
    // Can cast to String on parse:
    // Serial.println(root["name"].as<String>());

    if (root["command"] == "LEDstartHue") {
        Serial.println(">>> DING DING! We have a LEDstartHue command!");
        Serial.println(root["value"].as<String>());
        int targetHue = root["value"];
        CHSV tempColour = CHSV(targetHue, ledsHSV[0].sat, ledsHSV[0].val);
        // Serial.print("Hue: ");
        // Serial.println(tempColour.h);
        // Serial.print("Sat: ");
        // Serial.println(tempColour.s);
        // Serial.print("Val: ");
        // Serial.println(tempColour.v);
        // Serial.println(ledsHSV[0].sat);
        ledsHSV[0] = tempColour;
        leds[0] = tempColour;
        FastLED.show();
    }

    if (root["command"] == "setBrightness") {
        Serial.println(">>> setBrightness command received!");
        int targetBrightness = root["value"];
        CHSV tempColour = CHSV(ledsHSV[0].hue, ledsHSV[0].sat, targetBrightness);
        // Serial.print("Hue: ");
        // Serial.println(tempColour.h);
        // Serial.print("Sat: ");
        // Serial.println(tempColour.s);
        // Serial.print("Val: ");
        // Serial.println(tempColour.v);
        ledsHSV[0] = tempColour;
        leds[0] = tempColour;
        FastLED.show();
    }

    if (root["command"] == "servo1position") {
        float targetPosition = root["value"];
        servo1target = targetPosition;
        servo1.write(targetPosition);
        delay(20);
    }

    if (root["command"] == "servo2position") {
        float targetPosition = root["value"];
        servo2target = targetPosition;
        servo2.write(targetPosition);
        delay(20);
    }

    // We've handled all the possible commands, so we'll clear the JSON buffer:
    // This might solve the situation where the Skutter craps out and stops
    // responding to messages. Or it might prevent parsing of multiple messages arriving in rapid succesion.
    StaticJsonBuffer<1024> jsonBuffer;
    // Boom! It's gone.
    
}
