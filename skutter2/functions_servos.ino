void updateServos() {
    for (int i = 0; i < SERVO_COUNT; i++) {
        //try the mapping function here too
        servoPosition[i][0] = (float)map(time_current, time_start,
                                  time_end,(int)servoPosition[i][transitionStart],
                                  (int)servoPosition[i][transitionTarget]);

        // servoPosition[i][0] = (float)interpolate((int)servoPosition[i][transitionStart],
        //                           (int)servoPosition[i][transitionTarget],
        //                           time_start,
        //                           time_end,
        //                           time_current);
    }
    servo1.write(servoPosition[0][0]);
    servo2.write(servoPosition[1][0]);
}