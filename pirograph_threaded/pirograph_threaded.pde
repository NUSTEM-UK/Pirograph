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
int doneCount = 0;
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
  thread("processB");
  thread("processC");
  thread("processD");
}

void draw() {

  // iterate over the image processing threads and count how many are done
  doneCount = 0;
  for (int i = 0; i < NUMPORTS; i++) {
    doneCount += int(DONE[i]);
  }

  // Have all threads completed?
  if (doneCount == NUMPORTS) {
    // All threads complete

    // RENDER IN TURN & BROADCAST RESULTS
    // Render needs to happen on the main thread. Doing it all at once here doesn't hurt frame rates too badly,
    // and makes the joins between the quadrants look neat.
    for (int f = 0; f < NUMPORTS; f++) {
      // Access the relevant PGraphcs buffer.
      buffers[f].beginDraw();
      // Push the current geometry so we can manipulate it for rotated drawing
      buffers[f].pushMatrix();
      // Move origin to centre of surface
      buffers[f].translate(cam_width/2, cam_width/2);
      // Dial in the rotation angle
      buffers[f].rotate(radians(angle));
      // Now paste over in the new image
      buffers[f].image(intermediates[f], -cam_width/2, -cam_height/2, cam_width, cam_height);
      // We're done with the image transform, and with drawing to this buffer
      // Copy the buffer to the composites array
      // I'd expect this to be done outside of the geometry transform, but that seems to 
      // (a.) barf, with the image in the wrong place, and (b.) worse than halve the frame rate.
      // So we'll do it here. Because reasons.
      composites[f].copy(buffers[f], 0, 0, cam_width, cam_height, 0, 0, cam_width, cam_height);
      // *Now* we can back out of the geometry transform. Apparently.
      buffers[f].popMatrix();
      // ..and we're done with rendering to this buffer.
      buffers[f].endDraw();

      // Fire the new frame out to the UDP port
      // TODO: Do this when we return to the individual thread?
      // broadcast(composites[f], i);
    }
    
    // DRAW
    // Draw each of the offscreen buffers over the display window
    for (int i = 0; i < NUMPORTS; i++) {
      image(buffers[i], 0, 0, cam_width, cam_height);
    }

    // UPDATE FPS DISPLAY
    current_time = millis();
    fps = framesProcessed / ((current_time-start_time)/1000);
    println("Frame: ", framesProcessed, " fps: ", fps);
    framesProcessed++;

    // GET NEXT FRAME
    cam.read();

    // Update the rotation angle
    angle += angleStep;

    // Tell the threads they're no longer done.
    for (int i = 0; i < NUMPORTS; i++) {
      DONE[i] = false;
    }

  } else {
    // around we go.
  }

  // // Work out which buffer to render to main screen.
  // if (displayPortTarget != displayPort) {
  //   // Erase the previous image
  //   background(0);
  //   // Make the transition
  //   displayPort = displayPortTarget;
  // }
  // // Now blit the surface
  // image(buffers[displayPort], 0, 0);



  // for (int i = 0; i < NUMPORTS; i++) {
  //   composites[NUMPORTS].blend(composites[i], 0, 0, width, height, 0, 0, width, height, BLEND);
  // }
  // composites[NUMPORTS].updatePixels();
  // image(composites[NUMPORTS], 0, 0);

  // Store the current frame - use for saving images
  // composite = get()

} // END OF draw()


void processImage(int f) {
  // MAIN IMAGE PROCESSING LOOP
  // Invoked by thread handlers when a new frame is ready for processing.
  // Updates intermediates[] with a masked frame.

  // Set up our drawing surfaces
  intermediates[f] = cam.get();
  intermediates[f].loadPixels();
  maskImages[f].loadPixels();
  composites[f].loadPixels();
  
  // Now compute the image mask for the region in which we're interested
  for (int x = regions[f][0]; x < regions[f][2]; x++) {
    for (int y = regions[f][1]; y < regions[f][3]; y++) {
      int loc = x + y*cam_width;

      // In some versions, I've copied the image pixels here while computing the mask.
      // This doesn't seem efficient, hence the cam.get() line above.
      // ...but I'm leaving the line in in case I had reasons previously. It's been a while.
      // intermediates[f].pixels[loc] = color(255, 0, 0);
      
      // Find luminosity of current pixel (and cast to int)
      // Doing a 'correct' YUV transform
      Y = int((0.2126*red(intermediates[f].pixels[loc])) + (0.7152*green(intermediates[f].pixels[loc])) + (0.0722*blue(intermediates[f].pixels[loc])));
      // Quick version averaging RGB values, left in but commented out. Performance seems OK via the 'correct' approach.
      // Y = int((red(cam.pixels[loc]) + green(cam.pixels[loc]) + blue(cam.pixels[loc])) / 3.0);

      // Now use the computer luminosity to build the mask image, with cutoffs
      if (Y > threshold_high) {
        maskImages[f].pixels[loc] = color(0, 0, 255); // Fully opaque mask
      } else if (Y < threshold_low) {
        maskImages[f].pixels[loc] = color(0, 0, 0); // Fully transparent mask
      } else {
        // Apply a mask opacity on an interpolation between low and high Y values (between cutoffs).
        // This feathers the edges of the mask and looks a bit nicer.
        maskImages[f].pixels[loc] = color(0, 0, map(Y, threshold_low, threshold_high, 0, 255));
      }
    }
  }
  // Apply the mask image to the intermediate image (which came from the camera, remember)
  // Mask: https://processing.org/reference/PImage_mask_.html
  intermediates[f].mask(maskImages[f]);
  intermediates[f].updatePixels();  // Write the pixels back to the PImage. Not necessary?

  // At this point we're done: intermediates[f] stores the new frame with mask, to composite over the old frame.
  // So let's do the composite.
  // Unfortunately, rotate() only works when drawing a PImage, so we need to render to an 
  // offscreen buffer surface as an intermediate step.
  // ...and the drawing doesn't work on a background thread (Java error & impressively big crash)
  // so the drawing has to be in draw(). Gnats.
  
  // ...so we're done. We'll return to the thread handler which will set the DONE semaphore for this segment.

}

// Thread handlers. The heavy lifting is done in processImage()
void processA() {
  while (true) {
    if (!DONE[0]) {
      // broadcast the previous frame
      // broadcast(composites[0], 0);

      // and now handle the next one
      processImage(0);
      // mask processing done: signal the main thread that we're finished
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


