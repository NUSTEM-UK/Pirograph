/* Pirograph image processing

version: 3.0, 2018-04-25
Jonathan Sanderson for NUSTEM, Northumbria University
First exhibited at Maker Faire UK, April 2018.

Invoke with:

  processing-java  --sketch=[path to directory] --run --args [portnumber]


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

import mqtt.*;

IPCapture cam;

int NUMPORTS = 4;
int THISPORT = 0; // Defines with which channel (quadrant) we're working.

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
int[] clientPorts = {9100, 9101, 9102, 9103};
DatagramSocket ds0;
DatagramSocket ds1;
DatagramSocket ds2;
DatagramSocket ds3;
// Streaming targets - string representations of IP addresses
String[] streamTargets = { "10.0.1.15", "10.0.1.16", "10.0.1.17", "10.0.1.18" };

// MQTT client. Which you could probably work out from the class name. Great comment, Jonathan.
MQTTClient client;

float angle = 0;
float angleStep = 0.5;

int Y;

float threshold_low = 70;
float threshold_high = 255;

int start_time;
int current_time;
float fps;
int framesProcessed = 0;

// String saveFilePath = "/Users/jonathan/Desktop/";
String saveFilePath = "/Users/rygp8/Desktop/";
String filename;

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
  size(1920, 1080, P2D);

  // TODO: https://forum.processing.org/two/discussion/3013/are-undecorated-frames-dead-with-processing-2-x
  // ...which might give us undecorated windows. Which would be nice. Though I don't know if we can drag them. Hmm.

  // Have we been passed a port number?
  if (args != null) {
    // yes - assign it
    THISPORT = int(args[1]);
  } else {
    // no - default to port A
    println("args == null");
    THISPORT = 0;
  }
  println(">>> HANDLING PORT: ", THISPORT);
  frameRate(30);
  frame.setResizable(true);
  // pixelDensity(displayDensity()); // Retina display
  background(0,0,0);

  int start_time = millis();

  // Initialise images
  for (int i = 0; i < NUMPORTS+1; i++) {
    intermediates[i] = createImage(cam_width, cam_height, ARGB);
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
  
  // MQTT setup: connect and subscribe to the /reset topic
  client = new MQTTClient(this);
  client.connect("mqtt://10.0.1.3", "pirographdisplay");
  client.subscribe("pirograph/#");

  // UDP streaming setup
  setupDatagramSockets();

} // setup()

void draw() {

  if (DONE == false) {
      processImage(THISPORT);
  } else {
    // WE HAVE A FRAME
    // software rotate of surface
    // See: https://www.processing.org/tutorials/transform2d/
    pushMatrix(); // Save the current coordinate system
    translate(width/2, height/2); // Shift coordinate origin to centre screen
    rotate(radians(angle));
    image(intermediates[THISPORT], -cam_width/2, -cam_height/2, cam_width, cam_height);
    popMatrix(); // Revert coordinate origin. Would happen at the end of draw() anyway.
    angle += angleStep; // Increment rotation angle

    // capture the current display
    composites[THISPORT] = get();
    // Send a downscale to frame consumers over UDP
    broadcast(composites[THISPORT], THISPORT);

    current_time = millis();
    fps = framesProcessed / ((current_time-start_time)/1000);
    if (frameCount % 60 == 0) {
      // Output fps diagnostics every few seconds only.
      println("Frame: ", framesProcessed, " fps: ", fps);
    }
    framesProcessed++;

    DONE = false;
  }
}

void processImage(int f) {
  // if (cam.isAvailable() == true) { // Removed for performance: risky but gains half an fps, roughly.
    cam.read();
  
    intermediates[f] = cam.get(); // Copy camera image to intermediate
    intermediates[f].loadPixels(); // loadPixels may not be necessary, but it's supposed to be done. Apparently.
    maskImages[f].loadPixels();
    composites[f].loadPixels();

    // for (int x = 0; x < cam_width; x++) {
    //   for (int y = 0; y < cam_height; y++) {
    for (int x = regions[f][0]; x < regions[f][2]; x++) {
      for (int y = regions[f][1]; y < regions[f][3]; y++) {
        int loc = x + y*cam_width;
        
        // In some versions, I've copied the image pixels here while computing the mask.
        // This doesn't seem efficient, hence the cam.get() line above.
        // ...but I'm leaving the line in in case I had reasons previously. It's been a while.
        // intermediates[f].pixels[loc] = color(255, 0, 0);

        // Find luminosity of current pixel (cast to int)
        // Doing a 'correct' YUV transform
        Y = int((0.2126*red(cam.pixels[loc])) + (0.7152*green(cam.pixels[loc])) + (0.0722*blue(cam.pixels[loc])));
        // Quick version averaging RGB values, left in but commented out. Performance seems OK via the 'correct' approach.
        // Y = int((red(cam.pixels[loc]) + green(cam.pixels[loc]) + blue(cam.pixels[loc])) / 3.0);

        // Now use the computer luminosity to build the mask image, with cutoffs
        if (Y > threshold_high) {
          maskImages[f].pixels[loc] = color(0, 0, 255);
        } else if (Y < threshold_low) {
          maskImages[f].pixels[loc] = color(0, 0, 0);
        } else {
          // Apply a mask opacity on an interpolation between low and high Y values (between cutoffs).
          // This feathers the edges of the mask and looks a bit nicer.
          maskImages[f].pixels[loc] = color(0, 0, map(Y, threshold_low, threshold_high, 0, 255));
        }
      }
    }

    // Mask: https://processing.org/reference/PImage_mask_.html
    // Can use an integer array as mask.
    intermediates[f].mask(maskImages[f]);
    intermediates[f].updatePixels();

    // At this point we're done: intermediates[f] stores the new frame with mask, to composite over the old frame.
    // So let's do the composite.
    DONE = true;
  // }
}
