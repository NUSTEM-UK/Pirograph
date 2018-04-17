#include <FastLED.h>

#define PIN_PIXEL D1

#define PIXEL_COUNT 9
int targetBrightness;
CRGB leds[PIXEL_COUNT];
CHSV ledsHSV[PIXEL_COUNT][3];
CHSV tempColour = CHSV(0, 255, targetBrightness);



uint8_t transitionTarget; // to which index are we heading?
uint8_t transitionStart;  // from which index did we start?
uint32_t transitionTime;
uint32_t time_start;     // Start time of commanded transition
uint32_t time_end; // End time of commanded transition
uint32_t time_current;   // Recalculated in transition loop

void setup() {
    Serial.begin(115200);
    targetBrightness = 255;
    FastLED.addLeds<WS2811, PIN_PIXEL, GRB>(leds, PIXEL_COUNT);
//set the first LED 0
leds[0] = CHSV(0, 255, targetBrightness);
leds[PIXEL_COUNT-1] = CHSV(120, 255, targetBrightness);

    // for (int targetLED = 0; targetLED < PIXEL_COUNT; targetLED += 1) {
        
    //     leds[targetLED] = tempColour;
    //     ledsHSV[targetLED][0] = tempColour;
    //     ledsHSV[targetLED][1] = tempColour; // Also set state A

    //     tempColour = CHSV(120, 255, targetBrightness);
    //     ledsHSV[targetLED][2] = tempColour; // ...and state B
    // }
    FastLED.show();
    Serial.println(ledsHSV[0][0].hue);

    // Set up some defaults for the initial fun and games
    transitionTarget = 2; // to which index are we heading?
    transitionStart = 1;  // from which index did we start?
    transitionTime = 5000;
    time_start = millis();     // Start time of commanded transition
    time_end = time_start + transitionTime; // End time of commanded transition
    time_current = millis();
}
void loop(){


//     time_current = millis(); // When is now?
//     if (time_current < time_end) {
//         Serial.println("--- in transition");
//         // updateLEDs(); // Update the HSV arrays
//         // writeLEDs();  // Write the HSV values across to the RGB array
// } else {
//     Serial.println("We made it!");
//     delay(500);
//     // Update the transition times
//     time_start = time_current;
//     time_end = time_current + transitionTime;
// }
}