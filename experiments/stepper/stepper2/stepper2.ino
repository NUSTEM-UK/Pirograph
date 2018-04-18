#include <AccelStepper.h>
#define HALFSTEP 8

// Motor pin definitions
#define motorPin1  D5     // IN1 on the ULN2003 driver 1
#define motorPin2  D6     // IN2 on the ULN2003 driver 1
#define motorPin3  D7     // IN3 on the ULN2003 driver 1
#define motorPin4  D8     // IN4 on the ULN2003 driver 1

// Initialize with pin sequence IN1-IN3-IN2-IN4 for using the AccelStepper with 28BYJ-48
AccelStepper stepper1(HALFSTEP, motorPin1, motorPin3, motorPin2, motorPin4);

int currentTime;

int targetDur;

int endTime;

int stepperPositions[5];

void setup() {
      Serial.begin(115200);

  stepper1.setMaxSpeed(1000.0);
  stepper1.setAcceleration(100.0);
  stepper1.setSpeed(500);
  stepper1.moveTo(10000);
  targetDur = 5000;
  currentTime = millis();
  endTime = currentTime + targetDur;
  stepperPositions[0] = 1000;
  stepperPositions[1] = 500;
  stepperPositions[2] = 90;
  stepperPositions[3] = -90;
  //which way are we moving forward towards A (0), or towards B (1)
  stepperPositions[4] = 0;
  stepper1.setMaxSpeed(stepperPositions[0]);
  Serial.println(stepperPositions[0]);
  Serial.println(stepperPositions[1]);

  // set the speed to a default

}//--(end setup )---

void loop() {
  currentTime = millis();
  if (currentTime > endTime ){
    Serial.println("5!");
    if (stepperPositions[4] == 0){
      Serial.println("PONG!");
      stepperPositions[4] = 1;
      stepper1.setMaxSpeed(stepperPositions[1]);
      if (stepperPositions[2]>=0){
        stepper1.moveTo(+100000);
      } else {
        stepper1.moveTo(-100000);
      }
    }


    else {
      Serial.println("PING!");
      stepperPositions[4] = 0;
      stepper1.setMaxSpeed(stepperPositions[0]);
            if (stepperPositions[3]>=0){
        stepper1.moveTo(+100000);
      } else {
        Serial.println("Zap");
        stepper1.moveTo(-100000);
      }
    }
    endTime = currentTime + targetDur;
  }
  //Change direction when the stepper reaches the target position
  if (stepper1.distanceToGo() == 0) {
     stepper1.moveTo(stepper1.currentPosition()+5000);
  }
  stepper1.run();
 // Serial.println(stepper1.speed());

}
