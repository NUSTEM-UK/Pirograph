/* Pirograph image processing

version: 3.0, 2018-04-25
Jonathan Sanderson for NUSTEM, Northumbria University
First exhibited at Maker Faire UK, April 2018.


Receives MJPEG images streamed from MotionEyeOS camera, processes them, and serves the results
via UDP streams to individual Skutter control stations.

Also handles output for master installation display.

Makes use of Daniel Shiffman's UDP streaming example code:
  https://github.com/shiffman/Processing-UDP-Video-Streaming

*/

import ipcapture.*;       // For receiving frames from MotionEyeOS stream
import processing.video.*;// Possibly not needed?
import javax.imageio.*;   // This (and remaining imports) for UDP stream handling.
import java.awt.image.*;
import java.net.*;
import java.io.*;

IPCapture cam;

int NUMPORTS = 4;
boolean DONE = false;
// NUMPORTS+1 represents our composite image

// We're going to hold all the chunks in arrays.
// indices 0, 1, 2, 3 = quadrants A, B, C, D
// index 4 = 4-up composite
// index 5 = flat composite
PImage[] intermediates = new PImage[NUMPORTS+1];
PImage[] composites = new PImage[NUMPORTS+1];
PImage[] maskImages = new PImage[NUMPORTS+1];
PImage cameraImage;

int cam_width = 1640;
int cam_height = 922;



// UDP ports and sockets for streaming
int clientPortA = 9100;
int clientportB = 9101;
int clientPortC = 9202;
int clientPortD = 9203;
DatagramSocket dsA;
DatagramSocket dsB;
DatagramSocket dsC;
DatagramSocket dsD;

float angle = 0;
float angleStep = 0.5;

int Y;

float threshold_low = 70;
float threshold_high = 255;

int start_time;
int current_time;
float fps;
int framesProcessed = 0;

String saveFilePath = "/Volumes/outputs/";
String filename;

void setup() {
   size(1640, 922, P2D);
  //fullScreen(P2D, 1);
  frameRate(15);
  // frame.setResizable(true);
  // pixelDensity(displayDensity()); // Retina display
  background(0,0,0);

  int start_time = millis();

  // Initialise images
  for (int i = 0; i < NUMPORTS+1; i++) {
    intermediates[i] = createImage(cam_width, cam_height, RGB);
    composites[i] = createImage(cam_width, cam_height, ARGB);
    maskImages[i] = createImage(cam_width, cam_height, RGB);
  }
  
  // cam = new Capture(this, cam_width, cam_height, 30); // (parent, w, h, fps)
  // cam = new IPCapture(this, "http://192.168.0.33:8000/stream.mjpg", "", "");
  // cam = new IPCapture(this, "http://192.168.0.33:8081", "", "");
  cam = new IPCapture(this, "http://10.0.1.10:8081/", "", "");
  cam.start();
  cam.pixelWidth = cam_width;    // Explicit here to avoid weird scaling issues should we change resolution vs. display later.
  cam.pixelHeight = cam_height;
}

void draw() {

  if (DONE == false) {
      processImage(NUMPORTS);
  } else {
    // WE HAVE A FRAME
    // software rotate of surface
    // See: https://www.processing.org/tutorials/transform2d/
    pushMatrix(); // Save the current coordinate system
    translate(width/2, height/2); // Shift coordinate origin to centre screen
    rotate(radians(angle));
    image(intermediates[NUMPORTS], -width/2, -height/2);
    popMatrix(); // Revert coordinate origin. Would happen at the end of draw() anyway.
    angle += angleStep; // Increment rotation angle

    if (threshold_high > 255) {
      threshold_high = 255;
    }
    if (threshold_low < 0) {
      threshold_low = 0;
    }

    current_time = millis();
    fps = framesProcessed / ((current_time-start_time)/1000);
    println("Frame: ", framesProcessed, " fps: ", fps);
    framesProcessed++;

    DONE = false;

    // Store the current frame - use for saving images
    // composite = get();
  }
}

void processImage(int f) {
  // if (cam.isAvailable() == true) { // Removed for performance: risky but gains half an fps, roughly.
    cam.read();
  
    intermediates[f] = cam.get(); // Copy camera image to intermediate

    for (int x = 0; x < cam_width; x++) {
      for (int y = 0; y < cam_height; y++) {
        int loc = x + y*cam_width;
        
        // Find luminosity of current pixel (cast to int)
        Y = int((0.2126*red(cam.pixels[loc])) + (0.7152*green(cam.pixels[loc])) + (0.0722*blue(cam.pixels[loc])));
        // Y = int((red(cam.pixels[loc]) + green(cam.pixels[loc]) + blue(cam.pixels[loc])) / 3.0);

        if (Y > threshold_high) {
          // intermediates[f].pixels[loc] = cam.pixels[loc];
          maskImages[f].pixels[loc] = color(0, 0, 255);
        } else if (Y < threshold_low) {
          // intermediates[f].pixels[loc] = color(0, 0, 0);
          maskImages[f].pixels[loc] = color(0, 0, 0);
        } else {
          // intermediates[f].pixels[loc] = cam.pixels[loc];
          maskImages[f].pixels[loc] = color(0, 0, map(Y, threshold_low, threshold_high, 0, 255));
        }
      }
    }

    // Mask: https://processing.org/reference/PImage_mask_.html
    // Can use an integer array as mask.
    intermediates[f].mask(maskImages[f]);
    intermediates[f].updatePixels();
    DONE = true;
  // }
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
    intermediates[NUMPORTS] = createImage(cam_width, cam_height, RGB);
    image(intermediates[NUMPORTS], 0, 0);
    background(0);
  } else if (key == 'o') {
    angleStep += 0.25;
    println("Step angle: ", angleStep);
  } else if (key == 'l') {
    angleStep -= 0.25;
    println("Step angle: ", angleStep);
  } else if (key == 'O') {
    angle = 0;
    println("ANGLE RESET");
  } else if (key == 's') {
    filename = saveFilePath + "Pirograph-";
    filename += year()+"-"+month()+"-"+day()+"-"+hour()+"-"+minute()+"-"+second()+".png";
    saveFrame(filename);
    println(">>> FRAME SAVED!");
  }
  if (threshold_high > 255) {
    threshold_high = 255;
  }
  if (threshold_low < 0) {
    threshold_low = 0;
  }
}
