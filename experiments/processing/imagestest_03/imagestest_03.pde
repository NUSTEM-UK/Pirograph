// Basic thresholding to an RGBA destination
// This is doing the thresholding on RGB source pixels. Ugh.

import processing.video.*;

Capture cam;
PImage intermediate;
PImage composite;
PImage maskImage;

int cam_width = 1280;
int cam_height = 720;

int Y;

float threshold_low = 100;
float threshold_high = 200;

void setup() {
  size(1280, 720);
  background(0,0,0);

  // cam_width = width;
  // cam_width = height;
  
  String[] cameras = Capture.list();
  
  if (cameras.length == 0) {
    println("There are no available cameras.");
    exit();
  } else {
    println("Available cameras:");
    for (int i = 0; i < cameras.length; i++) {
      println(cameras[i]);
    }
  }
  
  intermediate = createImage(cam_width, cam_height, RGB);
  composite = createImage(cam_width, cam_height, ARGB);
  maskImage = createImage(cam_width, cam_height, RGB);
  cam = new Capture(this, cam_width, cam_height, 30); // (parent, w, h, fps)
  cam.start();
}

void draw() {
  
  if (cam.available() == true) {
    cam.read();
    cam.loadPixels();
    intermediate.loadPixels();
    // composite.loadPixels();
    maskImage.loadPixels();

    for (int x = 0; x < cam_width; x++) {
      for (int y = 0; y < cam_height; y++) {
        int loc = x + y*cam_width;
        
        // Find luminosity of current pixel (cast to int)
        Y = int((0.2126*red(cam.pixels[loc])) + (0.7152*green(cam.pixels[loc])) + (0.0722*blue(cam.pixels[loc])));

        if (Y > threshold_high) {
          intermediate.pixels[loc] = cam.pixels[loc];
          maskImage.pixels[loc] = color(0, 0, 255);
        } else if (Y < threshold_low) {
          intermediate.pixels[loc] = color(0, 0, 0);
          maskImage.pixels[loc] = color(0, 0, 0);
        } else {
          intermediate.pixels[loc] = cam.pixels[loc];
          maskImage.pixels[loc] = color(0, 0, Y);
        }
      }
    }

    // Mask: https://processing.org/reference/PImage_mask_.html
    // Can use an integer array as mask.
    intermediate.mask(maskImage);
    intermediate.updatePixels();
    image(intermediate, 0, 0);
    
    if (threshold_high > 255) {
      threshold_high = 255;
    }
    if (threshold_low < 0) {
      threshold_low = 0;
    }
    
    // Store the current frame.
    composite = get();
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
  }
  if (threshold_high > 255) {
    threshold_high = 255;
  }
  if (threshold_low < 0) {
    threshold_low = 0;
  }
}
  
/*
  // We're handling both images' pixels
  source.loadPixels();
  destination.loadPixels();
  
  for (int x = 0; x < source.width; x++) {
    for (int y = 0; y < source.width; y++) {
      int loc = x + y*source.width; // PImage arrays are 1-dimensional! Freaky!
      
      // Test brightness against threshold
      if (brightness(source.pixels[loc]) > threshold ) {
        destination.pixels[loc] = source.pixels[loc]; // White if color(255), but here copy over
      } else {
        destination.pixels[loc] = color(0,0,0,0);   // Black
      }
      
      //float r = red(img.pixels[loc]);
      //float g = green(img.pixels[loc]);
      //float b = blue(img.pixels[loc]);
      
      // Image processing here
      
      // Set the display pixel to the image pixel
      // pixels[loc] = color(r,g,b);
    }
  }
  destination.updatePixels();
  image(destination, 0, 0);
      
  //background(0);
  //tint(255, 127);
  //image(img, 200, 0);
  //tint(255, 100); // Can be (R, G, B, A)
  //image(img2, 250, 250);
}
*/