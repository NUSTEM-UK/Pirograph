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
        tempColour.sat = (int) map(time_current, time_start, time_end, ledsHSV[i][transitionStart].sat,
                                    ledsHSV[i][transitionTarget].sat;
        tempColour.val = (int) map(time_current, time_start, time_end, ledsHSV[i][transitionStart].val,
                                    ledsHSV[i][transitionTarget].val;
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

