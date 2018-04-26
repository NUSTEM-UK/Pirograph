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
#include <AccelStepper.h>

// Configurations
// Comment / uncomment as necessary.
// Commenting out STEPPER2 will enable SERVOS
#define STEPPER1 
#define STEPPER2 // NB. Collides with SERVOS
#define SERVO1
#define SERVO2

#define PIXEL_COUNT 9
#define SERVO_COUNT 2

#define HALFSTEP 8  // for steppers

#ifdef STEPPER1
    #define motorPin1  D5     // IN1 on the ULN2003 driver 1
    #define motorPin2  D6     // IN2 on the ULN2003 driver 1
    #define motorPin3  D7     // IN3 on the ULN2003 driver 1
    #define motorPin4  D8     // IN4 on the ULN2003 driver 1
    
    AccelStepper speedStepper(HALFSTEP, motorPin1, motorPin3, motorPin2, motorPin4);
#endif

#ifdef STEPPER2
    #define motorPin5  D4     // IN1 on the ULN2003 driver 1
    #define motorPin6  D2     // IN2 on the ULN2003 driver 1
    #define motorPin7  D1     // IN3 on the ULN2003 driver 1
    #define motorPin8  D0     // IN4 on the ULN2003 driver 1

    AccelStepper angleStepper(HALFSTEP, motorPin5, motorPin7, motorPin6, motorPin8);
#endif

#ifdef SERVO1
    #define PIN_SERVO1 D1
    #define PIN_SERVO2 D2
    Servo servo1;
    Servo servo2;   // if we have any servos, we have both servos
#endif



// Moving the Pixel strips to pin D3, for better stepper compatibility
// (doesn't work on D0, which would be the logical choice for board layout)
#define PIN_PIXEL D3


const char* ssid = "nustem";
const char* password = "nustem123";
const char* mqtt_server = "10.0.1.3";

WiFiClient espClient;
PubSubClient client(espClient);

// Each robot has a unique name, generated from the hardware MAC address.
// These variables will store those names.
char skutterNameArray[60];
String skutterNameString;
String subsTargetString;
char subsTargetArray[60];


// Store servo positions (current), A/B target states and transition states
float servoPosition[SERVO_COUNT][5];
int stepperSpeed[5];    // Need to work out which is a speed stepper
int stepperAngle[5];    // ...and which is an angle stepper
int nowSpeed;           // current speed of the speedy stepper

boolean transitionDirty = false;     // Do we have a pending target update at the end of this transition?

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
uint32_t time_start;      // Start time of commanded transition
uint32_t time_end;        // End time of commanded transition
uint32_t time_current;    // Recalculated in transition loop
String transitionType;    // Alternatives "LOOP", "RETURN"
String transitionInterpolation;

void setup() {
    Serial.begin(115200);
    setup_wifi();

    // Get this Huzzah's MAC address and use it to register with the MQTT server
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

    // Init the servo data
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            servoPosition[i][j] = 90.0;
        }
    }

    // Init the stepper data
    for (int i = 0; i < 5; i++){
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

    // set the peak speed of the speed stepper
    speedStepper.setMaxSpeed(1000);

    // diagnostics();
    Serial.println("END OF SETUP");
    Serial.println("----");

    #ifdef SERVO1
        servo1.attach(PIN_SERVO1);
        servo1.write(servoPosition[0][0]);
    #endif
    #ifdef SERVO2
        servo2.attach(PIN_SERVO2);
        servo2.write(servoPosition[1][0]);
    #endif
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
        //Serial.println("--- in transition");
        #ifdef SERVO1
            updateServos();
        #endif
        #ifdef STEPPER1
            updateSpeedStepper();
        #endif
        updateLEDs(); // Update the HSV arrays
        writeLEDs();  // Write the HSV values across to the RGB array
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
        #ifdef SERVO1
            servo1.write(servoPosition[0][0]);
            servo2.write(servoPosition[1][0]);
        #endif

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

    // Run the steppers, if appropriate
    #ifdef STEPPER1
        speedStepper.runSpeed();
    #endif
    #ifdef STEPPER2
        angleStepper.run();
    #endif

    // Make sure we show the results of all our fancy LED wrangling.
    FastLED.show();
    // delay(1000); // Slow things down so we can see what's going on.

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
