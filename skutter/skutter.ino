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

// const char* ssid = "nustem";
// const char* password = "nustem123";
// const char* mqtt_server = "10.0.1.3";

const char* ssid = "BadgerNet-2G";
const char* password = "Badgercwtch1";
const char* mqtt_server = "192.168.0.31";

WiFiClient espClient;
PubSubClient client(espClient);

// Each robot has a unique name, generated from the hardware MAC address.
// These variables will store those names.
char skutterNameArray[60];
//String huzzahMACAddress;
String skutterNameString;
String subsTargetString;
char subsTargetArray[60];

// Servo and LED strip setup
#define PIN_SERVO1 D7
#define PIN_SERVO2 D6
#define PIN_LED_BLUE D4
#define PIN_LED_RED D3
#define PIN_PIXEL D1

#define PIXEL_COUNT 1
#define SERVO_COUNT 2

Servo servo1;
Servo servo2;

// Store servo positions (current) and A/B target states
float servoPosition[SERVO_COUNT][3];


// Array of LEDs - current values
CRGB leds[PIXEL_COUNT];
// Need shadow array of HSV values, as the pixels are RGB objects but there's no
// conversion back to HSV.
// see: https://github.com/FastLED/FastLED/wiki/Pixel-reference
CHSV ledsHSV[PIXEL_COUNT][3];
int targetBrightness; // Really nasty kludge to lock this off.

// Transition setup
// A to B is index 1 to 2; B to A is 2 to 1.
uint8_t transitionTarget; // to which index are we heading?
uint8_t transitionStart;  // from which index did we start?
uint32_t transitionTime;
uint32_t time_start;     // Start time of commanded transition
uint32_t time_end; // End time of commanded transition
uint32_t time_current;   // Recalculated in transition loop
String transitionType; // Alternatives "LOOP", "RETURN"
String transitionInterpolation;

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

    targetBrightness = 100;

    FastLED.addLeds<WS2811, PIN_PIXEL, GRB>(leds, PIXEL_COUNT);
    for (int targetLED = 0; targetLED < PIXEL_COUNT; targetLED += 1) {
        CHSV tempColour = CHSV(0, 255, targetBrightness);
        leds[targetLED] = tempColour;
        ledsHSV[targetLED][0] = tempColour;
        ledsHSV[targetLED][1] = tempColour; // Also set state A
        ledsHSV[targetLED][2] = tempColour; // ...and state B
    }
    FastLED.show();

    // Enable the built-in LEDs for network diagnostics
    pinMode(PIN_LED_BLUE, OUTPUT);
    pinMode(PIN_LED_RED, OUTPUT);

    // Current positions
    // Could loop this, but this is a handy aide-memoir of what's what.
    servoPosition[0][0] = 90.0; // Servo1 current
    servoPosition[1][0] = 90.0; // Servo2 current
    servoPosition[0][1] = 90.0; // Servo1 state A
    servoPosition[0][2] = 90.0; // Servo1 state B
    servoPosition[1][1] = 90.0; // Servo2 state A
    servoPosition[1][2] = 90.0; // Servo2 state B

    // Set defaults
    transitionTarget = 2; // to which index are we heading?
    transitionStart = 1;  // from which index did we start?
    transitionTime = 5000;
    time_start = millis();     // Start time of commanded transition
    time_end = time_start + transitionTime; // End time of commanded transition
    time_current = millis();   // Recalculated in transition loop
    transitionType = "RETURN"; // Alternatives "LOOP", "RETURN", "ONCE"
    transitionInterpolation = "LINEAR"; // Not used. Yet.

    diagnostics();
    Serial.println("END OF SETUP");
    Serial.println("----");

    servo1.attach(PIN_SERVO1);
    servo1.write(servoPosition[0][0]);
    servo2.attach(PIN_SERVO2);
    servo2.write(servoPosition[1][0]);

}

void loop() {
    // Call the MQTT client to poll for updates, reconnecting if necessary
    // Handle messages via the callback function
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    time_current = millis(); // When is now?
    // Should we have already completed the current transition?
    if (time_current < time_end) {
        Serial.println("--- in transition");
        updateServos();
        updateLEDs(); // Update the HSV arrays
        writeLEDs();  // Write the HSV values across to the RGB array
        diagnostics();
    } else {
        // We should have completed transition by now.
        Serial.println("<<< TRANSITION COMPLETE");
        Serial.print("From start: ");
        Serial.print(transitionStart);
        Serial.print(" to target: ");
        Serial.println(transitionTarget);

        // Ensure we get there
        // write the transitionTarget values to current
        copyData(transitionTarget, 0);
        servo1.write(servoPosition[0][0]);
        servo2.write(servoPosition[1][0]);

        // Now reassign start and target states.
        if (transitionType == "ONCE") {
            // Set [transitionStart] data to that at [transitionTarget]
            copyData(transitionStart, transitionTarget);
            transitionStart = 2;
            transitionTarget = 2;
            // We can just leave it here, until we're needed again.
        } else if (transitionType == "LOOP") {
            // We're reverting to state A and heading for B again.
            copyData(transitionStart, 0);
            transitionStart = 1;
            transitionTarget = 2;
            time_end = time_current + transitionTime;
        } else if (transitionType == "RETURN") {
            // if we were going A to B, now set B to A
            // else set A to B
            if (transitionTarget = 2) {
                // Make the current state the target state
                copyData(2, 0);
                // Now reset the transition target
                transitionTarget = 1;
                transitionStart = 2;
                Serial.print("Heading to state A: ");
                Serial.println(transitionTarget);
            } else if (transitionTarget = 1) {
                // Make the current state the target state
                copyData(1, 0);
                // Now reset the transition target
                transitionTarget = 2;
                transitionStart = 1;
                Serial.print("Heading to state B: ");
                Serial.println(transitionTarget);
            }
            // Update the end time
            time_end = time_current + transitionTime;
        }

    }

    // Make sure we show the results of all our fancy LED wrangling.
    FastLED.show();
    delay(1000); // Slow things down so we can see what's going on.
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.println(">>> MQTT message received");
    // Serial.print("topic: ");
    // Serial.println(topic);
    // Serial.print("payload: ");
    // Serial.println(*payload);

    String payloadString;
    for (int i = 0; i < length ; i++) {
        payloadString += String((char)payload[i]);
    }
    Serial.print("payload (string): ");
    Serial.println(payloadString);

    // Declare the JSON buffer here, so we never reuse a buffer
    // (it's destroyed when the callback is re-entered)
    // There's a chance a command won't be parsed, if the decode and action
    // isn't completed before the next message arrives. But the D1 is pretty quick,
    // and queing commands just a little from the controller should prevent that from
    // happening. So we'll leave it up to the controller.
    StaticJsonBuffer<256> jsonBuffer;
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

    if (root["command"] == "setLEDhue") {
        String position = root["position"];
        String state = root["state"];
        int targetHue = root["value"];
        // Output diagnostics to serial for debugging.
        Serial.print("Setting ");
        Serial.print(position);
        Serial.print(" LED in state: ");
        Serial.print(state);
        Serial.print(" to hue: ");
        Serial.println(targetHue);
        // Work out which end of the pixel string we're addressing
        int pixel_number;
        if (position == "start") {
            pixel_number = 0;
        } else {
            pixel_number = (PIXEL_COUNT - 1); // I think that's right. Probably.
        }
        // Are we writing to state A or B?
        int stateIndex;
        if (state == "A") {
            stateIndex = 1;
        } else {
            stateIndex = 2;
        }
        // Update the appropriate array entry:
        // Serial.print("Setting ");
        // Serial.print(pixel_number);
        // Serial.print(" LED in state: ");
        // Serial.print(stateIndex);
        // Serial.print(" to hue: ");
        // Serial.println(targetHue);
        ledsHSV[pixel_number][stateIndex].hue = targetHue;
        // Serial.println(ledsHSV[pixel_number][stateIndex].hue);
        // Now update the pixel hue interpolation for this state:
        updateLEDgradient(stateIndex);
        // Set the transition timer running
        time_current = millis();
        time_end = time_current + transitionTime;
    }

    if (root["command"] == "setBrightness") {
        Serial.println(">>> setBrightness command received!");
        targetBrightness = root["value"];
        // Loops through all states, update all brightnesses
        for (int state = 0; state < 3; state++) {
            for (int i = 0; i < PIXEL_COUNT; i++) {
                ledsHSV[i][state].val = targetBrightness;
            }
        }
    }

    if (root["command"] == "setServoPosition") {
        int servoNum = root["servoNum"];
        String state = root["state"];
        float targetPosition = root["angle"];
        // Output parsed structure to serial, for debugging
        Serial.print("Set Servo: ");
        Serial.print(servoNum);
        Serial.print(" in state: ");
        Serial.print(state);
        Serial.print(" to position: ");
        Serial.println(targetPosition);
        // Now act on it.
        int stateIndex;
        // TODO: find out why `if (state == "A")` doesn't work here
        // Something weird to do with String handling
        if (state == "A") {
            stateIndex = 1;
        } else {
            stateIndex = 2;
        }
        Serial.print("Servo: ");
        Serial.print(servoNum);
        Serial.print(" State: ");
        Serial.print(stateIndex);
        Serial.print(" to position: ");
        Serial.println(targetPosition);
        servoPosition[servoNum-1][stateIndex] = targetPosition;
        Serial.print("Target servo set to : ");
        Serial.println(servoPosition[servoNum-1][stateIndex]);
        diagnostics();
        Serial.println("<<< END PROCESSING setServoPosition");
        // Set the transition timer running again
        time_current = millis();
        time_end = time_current + transitionTime;
    }

    if (root["command"] == "setTransitionType") {
        String tempType = root["value"];
        // Output parsed structure to serial, for debugging
        Serial.print("Transition Type: ");
        Serial.println(tempType);
        // Now act on it.
        transitionType = tempType;
    }

    if (root["command"] == 'setTransitionTime') {
        int transitionTime = root["value"];
        Serial.print("Transition time: ");
        Serial.println(transitionTime);
    }

}


int interpolate(int start_value, int target_value, int start_time, int end_time, int current_time) {
    float start_value_float = (float) start_value;
    float target_value_float = (float) target_value;
    float calculated_value_float;
    int calculated_value_int;

    // Serial.print(start_value_float);
    // Serial.print(" ");
    // Serial.println(target_value_float);

    if ( target_value_float < start_value_float ) {
        calculated_value_float = start_value_float - ( ( (start_value_float - target_value_float) / (float)(end_time - start_time) ) * (float)(current_time - start_time) );
    } else {
        calculated_value_float = start_value_float + ( ( (target_value_float - start_value_float) / (float)(end_time - start_time) ) * (float)(current_time - start_time) );
    }

    calculated_value_int = (int) calculated_value_float;
    return calculated_value_int;
}

void writeLEDs() {
    // Map ledsHSV to leds (RGB), and update the pixel string.
    for (int i = 0; i < PIXEL_COUNT ; i++) {
        // Get the HSV colour
        CHSV tempColour = CHSV(ledsHSV[i][0].hue, ledsHSV[i][0].sat, ledsHSV[i][0].val);
        // Now assign that to the RGB array; conversion is automatic
        leds[i] = tempColour;
    }
    // We'll need to call FastLED.show(); to update the string.
    // FastLED.show(); // ...but do that in the loop.
}

void updateLEDgradient(int state) {
    // interpolate HSV values between start and end points in string
    // Get the end points
    CHSV startColour = CHSV(ledsHSV[0][state].hue, ledsHSV[0][state].sat, ledsHSV[0][state].val);
    CHSV endColour = CHSV(ledsHSV[PIXEL_COUNT][state].hue, ledsHSV[PIXEL_COUNT][state].sat, ledsHSV[PIXEL_COUNT][state].val);
    // Now looping between the 2nd and n-1th LEDs
    for (int i = 1; i < (PIXEL_COUNT - 1); i++) {    
        CHSV tempColour;
        tempColour.hue = interpolate(startColour.hue, endColour.hue, 0, PIXEL_COUNT, i);
        tempColour.sat = interpolate(startColour.sat, endColour.sat, 0, PIXEL_COUNT, i);
        tempColour.val = interpolate(startColour.val, endColour.val, 0, PIXEL_COUNT, i);

        ledsHSV[i][state] = tempColour;
    }
}

void updateLEDs() {
    // Current state is ledsHSV[i][0], start is [i][transitionStart], target [i][transitionTarget]

    for (int i = 0; i < PIXEL_COUNT; i++) {
        CHSV tempColour;
        tempColour.hue = interpolate(ledsHSV[i][transitionStart].hue,
                                     ledsHSV[i][transitionTarget].hue,
                                     time_start, time_end, time_current);
        tempColour.sat = 255;
        tempColour.val = targetBrightness;
        // tempColour.sat = interpolate(ledsHSV[i][transitionStart].sat,
        //                              ledsHSV[i][transitionTarget].sat,
        //                              time_start, time_end, time_current);
        // tempColour.val = interpolate(ledsHSV[i][transitionStart].val,
        //                              ledsHSV[i][transitionTarget].val,
        //                              time_start, time_end, time_current);
        ledsHSV[i][0] = tempColour;
        leds[i] = tempColour;
        Serial.print(ledsHSV[i][transitionStart].hue);
        Serial.print(" ");
        Serial.print(ledsHSV[i][transitionTarget].hue);
        Serial.print(" ");
        Serial.println(tempColour.hue);
    }
    // Again, we'll need to call FastLED.show() to update the string. Do that in the loop.
}

void updateServos() {
    for (int i = 0; i < SERVO_COUNT; i++) {
        servoPosition[i][0] = interpolate(servoPosition[i][transitionStart],
                                  servoPosition[i][transitionTarget],
                                  time_start,
                                  time_end,
                                  time_current);
    }
    servo1.write(servoPosition[0][0]);
    servo2.write(servoPosition[1][0]);
}

void diagnostics() {
    for (int i = 0; i < PIXEL_COUNT; i++) {
        Serial.print("LED: ");
        Serial.print(i);
        Serial.print(" :: ");
        for (int j = 0; j < 3 ; j++) {
            Serial.print("(");
            Serial.print(ledsHSV[i][j].hue);
            Serial.print(",");
            Serial.print(ledsHSV[i][j].sat);
            Serial.print(",");
            Serial.print(ledsHSV[i][j].val);
            Serial.print(") ");
        }
        Serial.println();
    }
    Serial.println("Servos: ");
    for (int i = 0; i < 2; i++) {
        Serial.print("  ");
        Serial.print(i);
        Serial.print(": ");
        for (int j = 0; j < 3; j++) {
            Serial.print(servoPosition[i][j]);
            Serial.print(" ");
        }
        Serial.println();
    }
}

void copyData(int source, int destination) {
    // Copy data from source index to target index

    // First the LEDs
    for (int i = 0; i < PIXEL_COUNT; i++) {
            ledsHSV[i][destination] = ledsHSV[i][source];
    }
    // Now the Servos
    for (int i = 0; i < SERVO_COUNT; i++) {
        servoPosition[i][destination] = servoPosition[i][source];
    }
}