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
    println("*** RESET ***");
    // Take off and nuke the entire buffer from orbit, it's the only way to be sure.
    background(0);
    // image(intermediates[THISPORT], 0, 0);
  } else if (key == 'o') {
    angleStep += 0.05;
    println("Step angle: ", angleStep);
  } else if (key == 'l') {
    angleStep -= 0.05;
    println("Step angle: ", angleStep);
  } else if (key == 'O') {
    angle = 0;
    println("ANGLE RESET");
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