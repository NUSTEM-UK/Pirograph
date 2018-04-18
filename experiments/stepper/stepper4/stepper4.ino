#include <AccelStepper.h>
#define HALFSTEP 8

// Motor pin definitions
#define motorPin1  D5     // IN1 on the ULN2003 driver 1
#define motorPin2  D6     // IN2 on the ULN2003 driver 1
#define motorPin3  D7     // IN3 on the ULN2003 driver 1
#define motorPin4  D8     // IN4 on the ULN2003 driver 1

AccelStepper stepper1(HALFSTEP, motorPin1, motorPin3, motorPin2, motorPin4);

// timing details
int currentTime;
int targetDur;
int endTime;

// the stepper array
// the arrary looks like [speedA, speedB, transition]
// transition = 1 means we're travelling from A --> B
// transition = 0 means we're travelling from B --> A
int stepperInfo[3];
int stepperInfo2[3]; //the info that drives stepper 2, startAngle, endAngle, transition

// setup function
void setup() {
    Serial.begin(115200);
    stepper1.setMaxSpeed(1000.0);
    stepper1.setAcceleration(1000.0);
    //stepper1.setSpeed(100.0);
    //stepper1.setSpeed(500);
    // 4000 steps is just short of a full rotation
    // set the initial target to 0
    stepper1.moveTo(0);

    // set an initial duration and end time
    // the allowable minimun duration for the stepper one is 1sec, 
    // for stepper2 is 5 sec (0-360 deg)
    targetDur = 5000;
    currentTime = millis();
    endTime = currentTime + targetDur;

    //populate the stepperInfo array
    //speed maximum limited to 1000, otherwise the thang don't spin
    stepperInfo[0] = 1000; //speedA
    stepperInfo[1] = 500; //speedB
    stepperInfo[2] = 1; //transtion from A->B

    stepperInfo2[0] = 0; //angleA
    stepperInfo2[1] = 50; //angleB
    stepperInfo2[2] = 1; //transtion from A->B
}

void loop() {
    currentTime = millis();
    if (currentTime > endTime) {
        Serial.println("Time's up.");
        endTime = currentTime + targetDur;
        updateStepper1();
        //updateStepper2();
    }
    // send message to the stepper to do something
    stepper1.run();
}

void updateStepper2(){
    int totStepsA = map(stepperInfo2[0],0,360,0,4000);
    int totStepsB = map(stepperInfo2[1],0,360,0,4000);
    int deltaSteps = abs(totStepsA - totStepsB);
    int newSpeed = deltaSteps / ((targetDur/1000)-1);

    if (stepperInfo2[2] == 1){   // A->B
        Serial.println("B --> A");
        stepperInfo2[2] = 0;
        //map the angle to a number of steps (1 rev ~ 4000)
        Serial.print("Necessary speed: ");
        Serial.println(newSpeed);
        stepper1.setMaxSpeed(newSpeed);
        stepper1.setAcceleration(newSpeed);
        stepper1.moveTo(totStepsA);
    } else {
        Serial.println("A --> B");
        stepperInfo2[2] = 1;
        //map the angle to a number of steps (1 rev ~ 4000)
        Serial.print("Necessary speed: ");
        Serial.println(newSpeed);
        stepper1.setMaxSpeed(newSpeed);
        stepper1.setAcceleration(newSpeed);
        stepper1.moveTo(totStepsB);
    }
}

// function to shift stepper 1 - continuous rotation
void updateStepper1(){
        // check which transition we are doing
        if (stepperInfo[2] == 1){   // A->B
            // set current position as 0
            stepper1.setCurrentPosition(0.0);
            Serial.println("B --> A");
            stepperInfo[2] = 0;
            int newSpeed = abs(stepperInfo[0]); //absolute value of speedB
            float totSteps = (targetDur/1000 * newSpeed) - newSpeed;
            Serial.print("Number of steps: ");
            Serial.println(totSteps);
            stepper1.setMaxSpeed(newSpeed);
            stepper1.setAcceleration(newSpeed);
            if (stepperInfo[0]<=0){
                stepper1.moveTo(0-totSteps);
            } else {
            stepper1.moveTo(totSteps);
            }
        } else {                    // B -> A
            // set current position as 0
            stepper1.setCurrentPosition(0.0);

            Serial.print(stepperInfo[4]);
            Serial.println("A --> B");
            stepperInfo[2] = 1;     // 
            int newSpeed = abs(stepperInfo[1]); //absolute value of speedB
            float totSteps = (targetDur/1000 * newSpeed) - newSpeed;
            Serial.print("Number of steps: ");
            Serial.println(0-totSteps);
            stepper1.setMaxSpeed(newSpeed);
            stepper1.setAcceleration(newSpeed);
            if (stepperInfo[1]<=0){
                stepper1.moveTo(0-totSteps);
            } else {
            stepper1.moveTo(totSteps);
            }
        }    
}