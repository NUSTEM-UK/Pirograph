// Basic thresholding to an RGBA destination
// This is doing the thresholding on RGB source pixels. Ugh.

// import processing.video.*;
import ipcapture.*;

IPCapture cam;
PImage intermediate;
PImage composite;
PImage maskImage;
PImage cameraImage;

int cam_width;
int cam_height;

float angle = 0;
float angleStep = 0.5;

int Y;

float threshold_low = 70;
float threshold_high = 255;

int start_time;
int current_time;
float fps;
int framesProcessed = 0;

void setup() {
  size(1640, 922, P2D);
  frameRate(10); // We're not getting higher than this anyway.
  pixelDensity(displayDensity()); // Retina display
  background(0,0,0);

  cam_width = 1640;
  cam_height = 922;
  
  int start_time = millis();

  intermediate = createImage(cam_width, cam_height, RGB);
  composite = createImage(cam_width, cam_height, ARGB);
  maskImage = createImage(cam_width, cam_height, RGB);
  // cam = new Capture(this, cam_width, cam_height, 30); // (parent, w, h, fps)
   //cam = new IPCapture(this, "http://192.168.0.33:8000/stream.mjpg", "", "");
  //cam = new IPCapture(this, "http://192.168.0.33:8081", "", "");
  cam = new IPCapture(this, "http://10.0.1.10:8081/", "", "");
  cam.start();
  cam.pixelWidth = cam_width;    // Explicit here to avoid weird scaling issues should we change resolution vs. display later.
  cam.pixelHeight = cam_height;
}

void draw() {
  
  if (cam.isAvailable() == true) {
    cam.read();
    intermediate = cam.get();
    //cam.loadPixels();
    //intermediate.loadPixels();
    // composite.loadPixels();
    //maskImage.loadPixels();

    for (int x = 0; x < cam_width; x++) {
      for (int y = 0; y < cam_height; y++) {
        int loc = x + y*cam_width;
        
        // Find luminosity of current pixel (cast to int)
        Y = int((0.2126*red(cam.pixels[loc])) + (0.7152*green(cam.pixels[loc])) + (0.0722*blue(cam.pixels[loc])));
        // Y = int((red(cam.pixels[loc]) + green(cam.pixels[loc]) + blue(cam.pixels[loc])) / 3.0);

        if (Y > threshold_high) {
          //intermediate.pixels[loc] = cam.pixels[loc];
          maskImage.pixels[loc] = color(0, 0, 255);
        } else if (Y < threshold_low) {
          //intermediate.pixels[loc] = color(0, 0, 0);
          maskImage.pixels[loc] = color(0, 0, 0);
        } else {
          //intermediate.pixels[loc] = cam.pixels[loc];
          maskImage.pixels[loc] = color(0, 0, Y);
        }
      }
    }

    // Mask: https://processing.org/reference/PImage_mask_.html
    // Can use an integer array as mask.
    intermediate.mask(maskImage);
    intermediate.updatePixels();
    
    // software rotate of surface
    // See: https://www.processing.org/tutorials/transform2d/
    pushMatrix(); // Save the current coordinate system
    translate(width/2, height/2); // Shift coordinate origin to centre screen
    rotate(radians(angle));
    image(intermediate, -width/2, -height/2); // This actually updates the screen, in case you were wondering.
    popMatrix(); // Revert coordinate origin. Would happen at the end of draw() anyway.
    angle += angleStep; // Increment rotation angle

    if (threshold_high > 255) {
      threshold_high = 255;
    }
    if (threshold_low < 0) {
      threshold_low = 0;
    }

    current_time = millis();
    //fps = frameCount / ((current_time-start_time)/1000);
    fps = framesProcessed / ((current_time-start_time)/1000);
    println("Frame: ", frameCount, " fps: ", fps);
    framesProcessed++;

    // Store the current frame - use for saving images
    // composite = get();
  }
}

// Handle threshold changes
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
    intermediate = createImage(cam_width, cam_height, RGB);
    image(intermediate, 0, 0);
  } else if (key == 'o') {
    angleStep += 0.25;
    println("Step angle: ", angleStep);
  } else if (key == 'l') {
    angleStep -= 0.25;
    println("Step angle: ", angleStep);
  } else if (key == 'O') {
    angle = 0;
    println("ANGLE RESET");
  }
  if (threshold_high > 255) {
    threshold_high = 255;
  }
  if (threshold_low < 0) {
    threshold_low = 0;
  }
}
