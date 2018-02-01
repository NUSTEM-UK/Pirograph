// Basic thresholding to an RGBA destination
// This is doing the thresholding on RGB source pixels. Ugh.

import processing.video.*;

Capture cam;
PImage source;
PImage destination;
PImage img2;

int cam_width = 640;
int cam_height = 360;

float threshold = 50;

void setup() {
  size(1024, 576);
  background(0,255,0);
  
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
  
  source = loadImage("wishing_well.jpeg"); // LoadImage serves as constructor for PImage object. Hmm.
  destination = createImage(cam_width, cam_height, ARGB);
  img2 = createImage(200, 200, RGB); // make a new blank image

  cam = new Capture(this, cam_width, cam_height, 30); // (parent, w, h, fps)
  cam.start();
}

void draw() {
  
  if (cam.available() == true) {
    cam.read();
    //image(cam, 0, 0, width, height);
    //image(cam, (width-cam_width)/2, (height-cam_height)/2);
    //image(destination, width/2 - cam_width/2, height/2 - cam_height);
    
    cam.loadPixels();
    destination.loadPixels();
    
    for (int x = 0; x < cam_width; x++) {
      for (int y = 0; y < cam_height; y++) {
        int loc = x + y*cam_width;
        
        if (brightness(cam.pixels[loc]) > threshold) {
          destination.pixels[loc] = cam.pixels[loc];
        } else {
          destination.pixels[loc] = color(0, 0, 0, 0);
        }
      }
    }
    destination.updatePixels();
    image(destination, (width-cam_width)/2, (height-cam_height)/2);
  }
}

//void keyReleased() {
//  if (key == ' ') destination.copy(cam, 0, 0, cam.width, cam.height, 0, 0, destination.width, destination.height);
//}
  
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