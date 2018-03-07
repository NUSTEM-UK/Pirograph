#include <Servo.h>

#define PIN_SERVO1 D7
#define PIN_SERVO2 D6
#define SERVO_COUNT 2

Servo servo1;
Servo servo2;

float servoPosition[SERVO_COUNT][3];

uint8_t transitionTarget; // to which index are we heading?
uint8_t transitionStart;  // from which index did we start?
uint32_t transitionTime;
uint32_t time_start;     // Start time of commanded transition
uint32_t time_end; // End time of commanded transition
uint32_t time_current;   // Recalculated in transition loop

String transitionType;

void setup() {
    Serial.begin(115200);

    servoPosition[0][0] = 90.0; // Servo1 current
    servoPosition[0][1] = 0.0; // Servo1 state A
    servoPosition[0][2] = 180.0; // Servo1 state B
    servoPosition[1][0] = 90.0; // Servo2 current
    servoPosition[1][1] = 0.0; // Servo2 state A
    servoPosition[1][2] = 180.0; // Servo2 state B

    transitionTarget = 2; // to which index are we heading?
    transitionStart = 1;  // from which index did we start?
    transitionTime = 5000;
    time_start = millis();     // Start time of commanded transition
    time_end = time_start + transitionTime; // End time of commanded transition
    time_current = millis();   // Recalculated in transition loop

    Serial.println("Initial servo states:");
    diagnostics();
    Serial.println("----END SETUP----");
    // This checks OK.

    transitionType = "RETURN";

    servo1.attach(PIN_SERVO1);
    servo1.write(servoPosition[0][0]);
    servo2.attach(PIN_SERVO2);
    servo2.write(servoPosition[1][0]);
}

void loop() {
    time_current = millis();

    if (time_current < time_end) {
        Serial.println("In transition");
        updateServos();
        diagnostics();
        // delay(500);
    } else {
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
            time_end = time_current + transitionTime;
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
                Serial.print("Heading to state A: ");
                Serial.println(transitionTarget);
            } else if (transitionTarget == 1) {
                // Make the current state the target state
                copyData(1, 0);
                // Now reset the transition target
                transitionTarget = 2;
                transitionStart = 1;
                Serial.print("Heading to state B: ");
                Serial.println(transitionTarget);
            }
            // Update the times
            time_start = time_current;
            time_end = time_current + transitionTime;
        }
    }
}

void updateServos() {
    for (int i = 0; i < SERVO_COUNT; i++) {
        Serial.print("Servo: ");
        Serial.print(i);
        Serial.print(" from: ");
        Serial.print(servoPosition[i][transitionStart]);
        Serial.print(" to: ");
        Serial.print(servoPosition[i][transitionTarget]);
        Serial.print(" with transition target: ");
        Serial.println(transitionTarget);

        // servoPosition[i][0] = servoPosition[i][transitionStart] + (
        //                       (
        //                           (servoPosition[i][transitionTarget] - servoPosition[i][transitionStart]) /
        //                           (time_end - time_start)
        //                       ) * (time_current - time_start) );

        // Serial.println(servoPosition[i][transitionTarget] - servoPosition[i][transitionStart]); // Range
        // Serial.println(time_end - time_start); // Duration

        servoPosition[i][0] = (float) interpolate((int)servoPosition[i][transitionStart],
                                  (int)servoPosition[i][transitionTarget],
                                  time_start,
                                  time_end,
                                  time_current);
    }
    servo1.write(servoPosition[0][0]);
    servo2.write(servoPosition[1][0]);
}

void diagnostics() {
    Serial.print("Time Start: ");
    Serial.print(time_start);
    Serial.print(" Time End: ");
    Serial.print(time_end);
    Serial.print(" Time Current: ");
    Serial.println(time_current);
    // Serial.print("LEDs: ");
    // for (int i = 0; i < 3 ; i++) {
    //     Serial.print(ledsHSV[0][i].hue);
    //     Serial.print(" ");
    // }
    // Serial.println();
    Serial.println("Servos: ");
    for (int i = 0; i < SERVO_COUNT; i++) {
        Serial.print("  ");
        Serial.print(i);
        Serial.print(": ");
        for (int j = 0; j < 3; j++) {
            Serial.print(servoPosition[i][j]);
            Serial.print(" ");
        }
        Serial.println();
    }
    Serial.println("----");
}

int interpolate(int start_value, int target_value, int start_time, int end_time, int current_time) {
    float v_start = (float) start_value;
    float v_target = (float) target_value;
    float t_start = (float) start_time;
    float t_end = (float) end_time;
    float t_current = (float) current_time;
    float calculated_value_float;
    int calculated_value_int;

    // Serial.print(v_start);
    // Serial.print(" ");
    // Serial.println(v_target);

    calculated_value_float = v_start + (( (v_target - v_start) / (t_end - t_start) ) * (t_current - t_start) );

    // if ( v_target < v_start ) {
    //     calculated_value_float = v_start - ( ( (v_start - v_target) / (float)(end_time - start_time) ) * (float)(current_time - start_time) );
    // } else {
    //     calculated_value_float = v_start + ( ( (v_target - v_start) / (float)(end_time - start_time) ) * (float)(current_time - start_time) );
    // }

    calculated_value_int = (int) calculated_value_float;
    return calculated_value_int;
}

void copyData(int source, int destination) {
    // Copy data from source index to target index

    // Now the Servos
    for (int i = 0; i < SERVO_COUNT; i++) {
        servoPosition[i][destination] = servoPosition[i][source];
    }
}