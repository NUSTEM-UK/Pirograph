#include <Stepper.h>

const int stepsPerRevolution = 32; // Guess / default
const int gearRatio = 64;

const int stepsPerOutputRevolution = stepsPerRevolution * gearRatio; // = 2048

// Note 1, 3, 2, 4 sequence here - not certain if that's correct.
Stepper myStepper(stepsPerRevolution, D5, D7, D6, D8);

int stepCount = 0;
int revolutionCount = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {


 
  for (int i = 0; i < (stepsPerRevolution * gearRatio); i++) {
    myStepper.step(1);
    stepCount++;
    delay(10); // Can't be much less than that; delay(1) crashes the ESP.
  }
  revolutionCount++;
  Serial.print("turns: ");
  Serial.println(revolutionCount);
  delay(1000);


  
}
