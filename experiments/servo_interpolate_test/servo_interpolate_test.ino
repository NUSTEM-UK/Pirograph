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

    servo1.attach(PIN_SERVO1);
    servo1.write(servoPosition[0][0]);
    servo2.attach(PIN_SERVO2);
    servo2.write(servoPosition[1][0]);
}

void loop() {
    time_current = millis();
    Serial.println(servoPosition[0][2]);
    Serial.println(servoPosition[1][2]);
    diagnostics(); //This doesn't check OK. Huh?
    updateServos();
    diagnostics();
    delay(500);
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
    float start_value_float = (float) start_value;
    float target_value_float = (float) target_value;
    float calculated_value_float;
    int calculated_value_int;

    // Serial.print(start_value_float);
    // Serial.print(" ");
    // Serial.println(target_value_float);

    calculated_value_float = start_value_float + ( ((target_value_float - start_value_float) / (float)end_time - (float)start_time) * ((float)current_time - (float)start_time) );

    // if ( target_value_float < start_value_float ) {
    //     calculated_value_float = start_value_float - ( ( (start_value_float - target_value_float) / (float)(end_time - start_time) ) * (float)(current_time - start_time) );
    // } else {
    //     calculated_value_float = start_value_float + ( ( (target_value_float - start_value_float) / (float)(end_time - start_time) ) * (float)(current_time - start_time) );
    // }

    calculated_value_int = (int) calculated_value_float;
    return calculated_value_int;
}