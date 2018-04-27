// Key control: handle threshold changes.
void keyReleased() {
  if (key == 'd') {
    threshold_low--;
    println("Threshold LOW: ", threshold_low);
  } else if (key == 'e') {
    threshold_low++;
    println("Threshold LOW: ", threshold_low);
  } else if (key == 'D') {
    threshold_low -= 10;
    println("Threshold LOW: ", threshold_low);
  } else if (key == 'E') {
    threshold_low += 10;
    println("Threshold LOW: ", threshold_low);
  } else if (key == 'f') {
    threshold_high--;
    println("Threshold HIGH: ", threshold_high);
  } else if (key == 'r') {
    threshold_high++;
    println("Threshold HIGH: ", threshold_high);
  } else if (key == 'F') {
    threshold_high -= 10;
    println("Threshold HIGH: ", threshold_high);
  } else if (key == 'R') {
    threshold_high += 10;
    println("Threshold HIGH: ", threshold_high);
  } else if (key == 'P') {
    color black = color(0, 0, 0);
    println("Reset: ", displayPort);
    intermediates[displayPort] = createImage(cam_width, cam_height, RGB);
    buffers[displayPort] = createGraphics(cam_width, cam_height, P2D);    // Nuke the offscreen surfaces from orbit, it's the only way to be sure.
    // for (int i = 0; i < NUMPORTS+1; i++) {
    //   println("Reset: ", i);
    //   intermediates[i] = createImage(cam_width, cam_height, RGB);
    //   buffers[i] = createGraphics(cam_width, cam_height, P2D);    // Nuke the offscreen surfaces from orbit, it's the only way to be sure.
    // }
  } else if (key == 'o') {
    angleStep += 0.005;
    println("Step angle: ", angleStep);
  } else if (key == 'l') {
    angleStep -= 0.005;
    println("Step angle: ", angleStep);
  } else if (key == 'O') {
    angle = 0;
    println("ANGLE RESET");
  } else if (key == '1') {
    println("SWITCH TO CHANNEL 1");
    displayPortTarget = 0;
    DONE[0] = false;
  } else if (key == '2') {
    println("SWITCH TO CHANNEL 2");
    displayPortTarget = 1;
    DONE[1] = false;
  } else if (key == '3') {
    println("SWITCH TO CHANNEL 3");
    displayPortTarget = 2;
    DONE[2] = false;
  } else if (key == '4') {
    println("SWITCH TO CHANNEL 4");
    displayPortTarget = 3;
    DONE[3] = false;
  }
  
  if (threshold_high > 255) {
    threshold_high = 255;
    println("Threshold HIGH: ", threshold_high);
  }
  if (threshold_low < 0) {
    threshold_low = 0;
    println("Threshold LOW: ", threshold_low);
  }

}