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
//#include <Servo.h> //Dont need in Skutter Stepper
#include <FastLED.h> // Using FastLED not NeoPixel, to gain HSV colour support

// here comes the hot-steppers
#include <AccelStepper.h>
#define HALFSTEP 8

//we're using nearly all 9 easily availbale pins ont he wemos - and the Neos need to not be on D0
// Motor pin definitions - Stepper Speed 
#define motorPin1  D5     // IN1 on the ULN2003 driver 1
#define motorPin2  D6     // IN2 on the ULN2003 driver 1
#define motorPin3  D7     // IN3 on the ULN2003 driver 1
#define motorPin4  D8     // IN4 on the ULN2003 driver 1

AccelStepper speedStepper(HALFSTEP, motorPin1, motorPin3, motorPin2, motorPin4);

// Motor pin definitions - Stepper Speed 
#define motorPin5  D4     // IN1 on the ULN2003 driver 1
#define motorPin6  D2     // IN2 on the ULN2003 driver 1
#define motorPin7  D1     // IN3 on the ULN2003 driver 1
#define motorPin8  D0     // IN4 on the ULN2003 driver 1

AccelStepper angleStepper(HALFSTEP, motorPin5, motorPin7, motorPin6, motorPin8);


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

// Servo and LED strip setup
// #define PIN_SERVO1 D7
// #define PIN_SERVO2 D6
// #define PIN_LED_BLUE D4 //I can't use these because I don't have enough pins
// #define PIN_LED_RED D3

//its the only one I've got left!
#define PIN_PIXEL D3

#define PIXEL_COUNT 9
//#define SERVO_COUNT 2


// Servo servo1;
// Servo servo2;

// Store servo positions (current) and A/B target states
//float servoPosition[SERVO_COUNT][3];

// the arrays to hold the 1st and 2nd stepper speeds and angles
int stepperSpeed[3];
int stepperAngle[3];
// the current speed of the speedy stepper
int nowSpeed;


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
    // pinMode(PIN_LED_BLUE, OUTPUT);
    // pinMode(PIN_LED_RED, OUTPUT);

    //Start speed and angles servo positions
    for (int i = 0; i < 3; i++){
        stepperSpeed[i] = 0;
        stepperAngle[i] = 0;
    }

    // Set defaults
    transitionTarget = 2; // to which index are we heading?
    transitionStart = 1;  // from which index did we start?
    transitionTime = 5000;
    time_start = millis();     // Start time of commanded transition
    time_end = time_start + transitionTime; // End time of commanded transition
    time_current = millis();   // Recalculated in transition loop
    transitionType = "RETURN"; // Alternatives "LOOP", "RETURN", "ONCE"
    transitionInterpolation = "LINEAR"; // Not used. Yet.

    // set the speed and acceleration rate for sped[0]
    speedStepper.setMaxSpeed(1000);
    //angleStepper.setMaxSpeed(1000);  

    //diagnostics();
    Serial.println("END OF SETUP");
    Serial.println("----");

    // servo1.attach(PIN_SERVO1);
    // servo1.write(servoPosition[0][0]);
    // servo2.attach(PIN_SERVO2);
    // servo2.write(servoPosition[1][0]);

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
        //updateServos();
        updateLEDs(); // Update the HSV arrays
        writeLEDs();  // Write the HSV values across to the RGB array
        updateSpeedStepper();
        //diagnostics(); // commented out diagnostics(), because no one needs this amount of data
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
        // servo1.write(servoPosition[0][0]);
        // servo2.write(servoPosition[1][0]);

        // Now reassign start and target states.
        if (transitionType == "ONCE") {
            // Set [transitionStart] data to that at [transitionTarget]
            Serial.println("*** ONCE ONLY");
            copyData(transitionStart, transitionTarget);
            transitionStart = 2;
            transitionTarget = 2;
            // We can just leave it here, until we're needed again.
        } else if (transitionType == "LOOP") {
            // We're reverting to state A and heading for B again.
            Serial.println("*** LOOPING");
            copyData(transitionStart, 0);
            transitionStart = 1;
            transitionTarget = 2;
        } else if (transitionType == "RETURN") {
            // if we were going A to B, now set B to A
            // else set A to B
            Serial.println("*** RETURNING");
            updateAngleStepper();
            if (transitionTarget == 2) {
                // Make the current state the target state
                copyData(2, 0);
                // Now reset the transition target
                transitionTarget = 1;
                transitionStart = 2;
                // Serial.print("Heading to state A: ");
                // Serial.println(transitionTarget);
            } else if (transitionTarget == 1) {
                // Make the current state the target state
                copyData(1, 0);
                // Now reset the transition target
                transitionTarget = 2;
                transitionStart = 1;
                // Serial.print("Heading to state B: ");
                // Serial.println(transitionTarget);
            }
        }
        // Update the transition times - note that transitions times only get updated once we reach the end of a loop
        Serial.println("Start time updated");
        time_start = time_current;
        // Serial.println(time_start);
        // Serial.print("End time updated: ");
        Serial.print("Transition Duration");
        Serial.print(transitionTime);
        time_end = time_current + transitionTime;
        // // Serial.println(time_end);
        // Serial.print("New duration: ");
        // Serial.println(time_end-time_start);

       

    }
    //Run the speedy stepper
    angleStepper.run();
    speedStepper.runSpeed();
    // Make sure we show the results of all our fancy LED wrangling.
    FastLED.show();
    // delay(1000); // Slow things down so we can see what's going on.

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
        // time_current = millis();
        // time_end = time_current + transitionTime;
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

    if (root["command"] == "setTransitionType") {
        String tempType = root["value"];
        // Output parsed structure to serial, for debugging
        Serial.print("Transition Type: ");
        Serial.println(tempType);
        // Now act on it.
        transitionType = tempType;
    }

    if (root["command"] == "setStepperAngle") {
        int angle = root["angle"];
        String state = root["state"];
        // Output parsed structure to serial, for debugging
        // Now act on it.
        if (state == "A") {
            stepperAngle[0] = angle;
        } else {
            stepperAngle[1] = angle;
        }
    }

    if (root["command"] == "setStepperSpeed") {
        int speed = root["speed"];
        String state = root["state"];
        // Now act on it.
        if (state == "A") {
            stepperSpeed[0] = speed;
                } else {
            stepperSpeed[1] = speed;
        }
    }

    if (root["command"] == "setTransitionTime") { //Comparisons here must use " " and not ''
        Serial.print("PING");
        transitionTime = root["value"];
        Serial.print("Transition time: ");
        Serial.println(transitionTime);
        // Set the transition timer running again - added this to see what goes on
        time_current = millis();
        time_end = time_current + transitionTime;
    }

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
    CHSV endColour = CHSV(ledsHSV[PIXEL_COUNT - 1][state].hue, ledsHSV[PIXEL_COUNT - 1][state].sat, ledsHSV[PIXEL_COUNT - 1][state].val);
    // Now looping between the 2nd and n-1th LEDs
    // for (int i = 1; i < PIXEL_COUNT; i++) {    
    //     CHSV tempColour;
    //     tempColour.hue = interpolate(startColour.hue, endColour.hue, 0, PIXEL_COUNT, i);
    //     tempColour.sat = interpolate(startColour.sat, endColour.sat, 0, PIXEL_COUNT, i);
    //     tempColour.val = interpolate(startColour.val, endColour.val, 0, PIXEL_COUNT, i);

    //     ledsHSV[i][state] = tempColour;
    // }

    // let's try this with the 'map' function - Syntax map(value, fromLow, fromHigh, toLow, toHigh)
    for (int i = 1; i < PIXEL_COUNT; i++) {    
        CHSV tempColour;
        tempColour.hue = (int) map(i, 0, PIXEL_COUNT, startColour.hue, endColour.hue);
        tempColour.sat = (int) map(i, 0, PIXEL_COUNT, startColour.sat, endColour.sat);
        tempColour.val = (int) map(i, 0, PIXEL_COUNT, startColour.val, endColour.val);

        ledsHSV[i][state] = tempColour;
    }
}

void updateLEDs() {
    // Current state is ledsHSV[i][0], start is [i][transitionStart], target [i][transitionTarget]

    for (int i = 0; i < PIXEL_COUNT; i++) {
        CHSV tempColour;
        // inserted the mp function here
        tempColour.hue = (int) map(time_current, time_start, time_end, ledsHSV[i][transitionStart].hue,
                                     ledsHSV[i][transitionTarget].hue
                                     );
        // tempColour.hue = interpolate(ledsHSV[i][transitionStart].hue,
        //                              ledsHSV[i][transitionTarget].hue,
        //                              time_start, time_end, time_current);
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
        // Serial.print(ledsHSV[i][transitionStart].hue);
        // Serial.print(" ");
        // Serial.print(ledsHSV[i][transitionTarget].hue);
        // Serial.print(" ");
        // Serial.println(tempColour.hue);
    }
    // Again, we'll need to call FastLED.show() to update the string. Do that in the loop.
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
}

void copyData(int source, int destination) {
    // Copy data from source index to target index
    // First the LEDs
    for (int i = 0; i < PIXEL_COUNT; i++) {
            ledsHSV[i][destination] = ledsHSV[i][source];
    }

}

void updateSpeedStepper(){
    if (stepperSpeed[2]==1){
    nowSpeed = map(time_current, time_start, time_end, stepperSpeed[0], stepperSpeed[1] );
    } else {
    nowSpeed = map(time_current, time_start, time_end, stepperSpeed[1], stepperSpeed[0] );    
    }
    speedStepper.setSpeed(nowSpeed);
}

void updateAngleStepper(){
    int totStepsA = map(stepperAngle[0],0,360,0,4000);
    int totStepsB = map(stepperAngle[1],0,360,0,4000);
    int deltaSteps = abs(totStepsA - totStepsB);
    int newSpeed = deltaSteps / ((transitionTime/1000)-1);

    if (stepperAngle[2] == 1){   // A->B
        Serial.println("B --> A");
        stepperAngle[2] = 0;
        //map the angle to a number of steps (1 rev ~ 4000)
        Serial.print("Necessary speed: ");
        Serial.println(newSpeed);
        angleStepper.setMaxSpeed(newSpeed);
        angleStepper.setAcceleration(newSpeed);
        angleStepper.moveTo(totStepsA);
    } else {
        Serial.println("A --> B");
        stepperAngle[2] = 1;
        //map the angle to a number of steps (1 rev ~ 4000)
        Serial.print("Necessary speed: ");
        Serial.println(newSpeed);
        angleStepper.setMaxSpeed(newSpeed);
        angleStepper.setAcceleration(newSpeed);
        angleStepper.moveTo(totStepsB);
    }
}
