void updateSpeedStepper(){
    if (stepperSpeed[2]==1){
    nowSpeed = map(time_current, time_start, time_end, stepperSpeed[0], stepperSpeed[1] );
    } else {
    nowSpeed = map(time_current, time_start, time_end, stepperSpeed[1], stepperSpeed[0] );    
    }
    speedStepper.setSpeed(nowSpeed);
}

void updateAngleStepper(){
    int totStepsA = map(stepperAngle[0],0,360,0,4000);
    int totStepsB = map(stepperAngle[1],0,360,0,4000);
    int deltaSteps = abs(totStepsA - totStepsB);
    int newSpeed = deltaSteps / ((transitionTime/1000)-1);

    if (stepperAngle[2] == 1){   // A->B
        Serial.println("B --> A");
        stepperAngle[2] = 0;
        //map the angle to a number of steps (1 rev ~ 4000)
        Serial.print("Necessary speed: ");
        Serial.println(newSpeed);
        angleStepper.setMaxSpeed(newSpeed);
        angleStepper.setAcceleration(newSpeed);
        angleStepper.moveTo(totStepsA);
    } else {
        Serial.println("A --> B");
        stepperAngle[2] = 1;
        //map the angle to a number of steps (1 rev ~ 4000)
        Serial.print("Necessary speed: ");
        Serial.println(newSpeed);
        angleStepper.setMaxSpeed(newSpeed);
        angleStepper.setAcceleration(newSpeed);
        angleStepper.moveTo(totStepsB);
    }
}


