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
volatile boolean[] DONE = new boolean [NUMPORTS+1];
int displayPort;
int displayPortTarget;
// Streaming targets - string representations of IP addresses
String[] streamTargets = { "10.0.1.15", "10.0.1.16", "10.0.1.17", "10.0.1.18" };

// We're going to hold all the chunks in arrays.
// indices 0, 1, 2, 3 = quadrants A, B, C, D
// index 4 = 4-up composite
// index 5 = flat composite
PImage[] intermediates = new PImage[NUMPORTS+1];
PImage[] composites = new PImage[NUMPORTS+1];
PImage[] maskImages = new PImage[NUMPORTS+1];
PImage cameraImage;

PGraphics[] buffers = new PGraphics[NUMPORTS+1];

int cam_width = 1640;
int cam_height = 922;

// UDP ports and sockets for streaming
int[] clientPorts = {9100, 9101, 9102, 9103};
DatagramSocket ds0;
DatagramSocket ds1;
DatagramSocket ds2;
DatagramSocket ds3;

float angle = 0;
float angleStep = 0.02;

int Y;

float threshold_low = 70;
float threshold_high = 255;

int start_time;
int current_time;
float fps;
int framesProcessed = 0;

// int[][] regions = new int[NUMPORTS+1][4]; // Will hold our image processing regions for the threads
// Initialise regions
int[][] regions = {
  { 0, 0, (cam_width/2), (cam_height/2) },
  { cam_width/2, 0, cam_width, (cam_height/2) },
  { cam_width/2, cam_height/2, cam_width, cam_height },
  { 0, cam_height/2, (cam_width/2), cam_height },
  { 0, 0, cam_width, cam_height }
};

void setup() {
  // fullscreen(P2D, 2) // Display full resolution on second display
  size(1640, 922, P2D);
  frameRate(30);
  pixelDensity(displayDensity()); // Retina display
  background(0,0,0);
  displayPort = 0;
  displayPortTarget = 0;

  int start_time = millis();

  // Initialise images
  for (int i = 0; i < NUMPORTS+1; i++) {
    DONE[i] = false;
    intermediates[i] = createImage(cam_width, cam_height, RGB);
    maskImages[i] = createImage(cam_width, cam_height, RGB);
    composites[i] = createImage(cam_width, cam_height, ARGB);
  }

  // Initialise PGraphics surfaces
  for (int i = 0; i < NUMPORTS+1; i++) {
    buffers[i] = createGraphics(cam_width, cam_height, P2D);
  }

  // cam = new Capture(this, cam_width, cam_height, 30); // (parent, w, h, fps)
  // cam = new IPCapture(this, "http://192.168.0.33:8000/stream.mjpg", "", "");
  // cam = new IPCapture(this, "http://192.168.0.33:8081", "", "");
  cam = new IPCapture(this, "http://10.0.1.10:8081/", "", "");
  cam.start();
  cam.pixelWidth = cam_width;    // Explicit here to avoid weird scaling issues should we change resolution vs. display later.
  cam.pixelHeight = cam_height;

  // UDP streaming setup
  setupDatagramSockets();

  for (int i = 0; i < NUMPORTS+1; i++) {
    print(i);
    print(" : ");
    for (int j = 0; j < 4; j++) {
      print(regions[i][j]);
      print(", ");
    }
    println();
  }

  // thread("processQuad");
  thread("processA");
  // thread("processB");
  // thread("processC");
  // thread("processD");
}

void draw() {

  // iterate over the PORT threads
  // TODO: run this in the image processing threads. Durr.
  for (int i = 0; i < NUMPORTS; i++) {
    // is the frame segment ready?
    if (DONE[i] == true) {
      composites[i].loadPixels();
      // yes, it's ready - so composite it in the appropriate offscreen buffer.
      buffers[i].beginDraw();
      buffers[i].pushMatrix();
      buffers[i].translate(width/2, height/2); // Shift coordinate origin to centre screen
      buffers[i].rotate(radians(angle));
      buffers[i].image(intermediates[i], -width/2, -height/2, width, height);
      // composites[i].copy(intermediates[i], -intermediates[i].width/2, -intermediates[i].height/2, intermediates[i].width, intermediates[i].height, 0, 0, intermediates[i].width, intermediates[i].height);
      // composites[i].copy(intermediates[i], 0, 0, intermediates[i].width, intermediates[i].height, 0, 0, intermediates[i].width, intermediates[i].height);
      buffers[i].popMatrix(); // Revert coordinate origin. Would happen at the end of draw() anyway.
      buffers[i].endDraw();
      composites[i].copy(buffers[i], 0, 0, cam_width, cam_height, 0, 0, cam_width, cam_height);
      broadcast(composites[i], i);
      // composites[i].copy(buffers[i], -width/2, -height/2, width, height, 0, 0, width, height);
      // composites[i].updatePixels();
      // image(intermediates[i], -width/2, -height/2); // This is where the display updates!
      // image(buffers[i], 0 , 0);
      DONE[i] = false;  // reset the semaphore so the thread restarts

      // current_time = millis();
      // fps = framesProcessed / ((current_time-start_time)/1000);
      // println("Frame: ", framesProcessed, " fps: ", fps);
      // framesProcessed++;
    }
    angle += angleStep; // Increment rotation angle
  }

  // Work out which buffer to render to main screen.
  if (displayPortTarget != displayPort) {
    // Erase the previous image
    background(0);
    // Make the transition
    displayPort = displayPortTarget;
  }
  // Now blit the surface
  image(buffers[displayPort], 0, 0);

  // TODO: Update this every n frames. I guess.
  // for (int i = 0; i < NUMPORTS; i++) {
  //   image(buffers[i], 0, 0);
  // }

  // for (int i = 0; i < NUMPORTS; i++) {
  //   composites[NUMPORTS].blend(composites[i], 0, 0, width, height, 0, 0, width, height, BLEND);
  // }
  // composites[NUMPORTS].updatePixels();
  // image(composites[NUMPORTS], 0, 0);

  // Store the current frame - use for saving images
  // composite = get()
}

void processImage(int f) {
  cam.read();
  intermediates[f] = cam.get(); // Copy camera image to intermediate
  intermediates[f].loadPixels();
  maskImages[f].loadPixels();
  for (int x = regions[f][0]; x < regions[f][2]; x++) {
    for (int y = regions[f][1]; y < regions[f][3]; y++) {
      int loc = x + y*cam_width;

      // intermediates[f].pixels[loc] = color(255, 0, 0);
      
      // Find luminosity of current pixel (cast to int)
      Y = int((0.2126*red(intermediates[f].pixels[loc])) + (0.7152*green(intermediates[f].pixels[loc])) + (0.0722*blue(intermediates[f].pixels[loc])));
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
  intermediates[f].mask(maskImages[f]);
  // intermediates[f].updatePixels();
}

// Thread handlers. The heavy lifting is done in processImage()
void processA() {
  while (true) {
    if (!DONE[0]) {
      processImage(0);
      DONE[0] = true;
    }
  }
}

void processB() {
  while (true) {
    if (!DONE[1]) {
      processImage(1);
      DONE[1] = true;
    }
  }
}

void processC() {
  while (true) {
    if (!DONE[2]) {
      processImage(2);
      DONE[2] = true;
    }
  }
}

void processD() {
  while (true) {
    if (!DONE[3]) {
      processImage(3);
      DONE[3] = true;
    }
  }
}

void processQuad() {
  println("processQuad started");
  while (true) {
    if (!DONE[NUMPORTS]) {
      processImage(NUMPORTS);
      DONE[NUMPORTS] = true;
    }
  }
}
// End thread handlers


