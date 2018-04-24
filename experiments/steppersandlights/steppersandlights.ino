#include <FastLED.h>
#include <AccelStepper.h>
#define HALFSTEP 8

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

#define PIN_PIXEL D3
#define PIXEL_COUNT 9

CRGB leds[PIXEL_COUNT];

void setup() {
    Serial.begin(115200);
    FastLED.addLeds<WS2811, PIN_PIXEL, GRB>(leds, PIXEL_COUNT);
    for (int i = 0; i < PIXEL_COUNT; i++){
        leds[i] = CRGB::Red;
    }
    FastLED.show();
    speedStepper.setMaxSpeed(1000);
    speedStepper.setSpeed(500);	
    angleStepper.setMaxSpeed(1000);
    angleStepper.setSpeed(500);	
}

void loop()
{  
speedStepper.runSpeed();
angleStepper.runSpeed();
}