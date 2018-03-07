#include <FastLED.h>

#define PIN_PIXEL D1
#define PIXEL_COUNT 1

CRGB leds[PIXEL_COUNT];
CHSV ledsHSV[PIXEL_COUNT][3];

uint8_t transitionTarget; // to which index are we heading?
uint8_t transitionStart;  // from which index did we start?
uint32_t transitionTime;
uint32_t time_start;     // Start time of commanded transition
uint32_t time_end; // End time of commanded transition
uint32_t time_current;   // Recalculated in transition loop

void setup() {
    Serial.begin(115200);

    FastLED.addLeds<WS2811, PIN_PIXEL, GRB>(leds, PIXEL_COUNT);
    for (int targetLED = 0; targetLED < PIXEL_COUNT; targetLED += 1) {
        CHSV tempColour = CHSV(0, 255, 255);
        leds[targetLED] = tempColour;
        ledsHSV[targetLED][0] = tempColour;
        ledsHSV[targetLED][1] = tempColour;
        ledsHSV[targetLED][2] = tempColour;
    }
    FastLED.show();

    ledsHSV[0][2].hue = 127;

    transitionTarget = 2; // to which index are we heading?
    transitionStart = 1;  // from which index did we start?
    transitionTime = 5000;
    time_start = millis();     // Start time of commanded transition
    time_end = time_start + transitionTime; // End time of commanded transition
    time_current = millis();   // Recalculated in transition loop

    Serial.println("Initial states:");
    diagnostics();
    Serial.println("----END SETUP----");
    // This checks OK.
}

void loop() {
    time_current = millis();
    updateLEDs();
    writeLEDs();
    diagnostics();
    // delay(500);
    FastLED.show();
}

void writeLEDs() {
    // Map ledsHSV to leds (RGB), and update the pixel string.
    for (int i = 0; i < PIXEL_COUNT ; i++) {
        // Get the HSV colour
        CHSV tempColour = CHSV(ledsHSV[i][0].hue, ledsHSV[i][0].sat, ledsHSV[i][0].val);
        // Now assign that to the RGB array; conversion is automatic
        Serial.println(tempColour.hue);
        leds[i] = tempColour;
    }
    // We'll need to call FastLED.show(); to update the string.
    // FastLED.show(); // ...but do that in the loop.
}

void updateLEDs() {
    // Current state is ledsHSV[i][0], start is [i][transitionStart], target [i][transitionTarget]

    for (int i = 0; i < PIXEL_COUNT; i++) {
        CHSV tempColour;
        tempColour.hue = interpolate(ledsHSV[i][transitionStart].hue,
                                     ledsHSV[i][transitionTarget].hue,
                                     time_start, time_end, time_current);
        tempColour.sat = interpolate(ledsHSV[i][transitionStart].sat,
                                     ledsHSV[i][transitionTarget].sat,
                                     time_start, time_end, time_current);
        tempColour.val = interpolate(ledsHSV[i][transitionStart].val,
                                     ledsHSV[i][transitionTarget].val,
                                     time_start, time_end, time_current);
        ledsHSV[i][0] = tempColour;
        leds[i] = tempColour;
    }
    // Again, we'll need to call FastLED.show() to update the string. Do that in the loop.
}

void diagnostics() {
    Serial.print("Time Start: ");
    Serial.print(time_start);
    Serial.print(" Time End: ");
    Serial.print(time_end);
    Serial.print(" Time Current: ");
    Serial.println(time_current);
    Serial.print("LEDs: ");
    for (int i = 0; i < 3 ; i++) {
        Serial.print("Selected hue: ");
        Serial.print(ledsHSV[0][i].hue);
        Serial.print(" ");
    }
    Serial.println();
    // Serial.println("Servos: ");
    // for (int i = 0; i < SERVO_COUNT; i++) {
    //     Serial.print("  ");
    //     Serial.print(i);
    //     Serial.print(": ");
    //     for (int j = 0; j < 3; j++) {
    //         Serial.print(servoPosition[i][j]);
    //         Serial.print(" ");
    //     }
    //     Serial.println();
    // }
    Serial.println("----");
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