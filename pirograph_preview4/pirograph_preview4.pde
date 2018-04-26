// Receiver / preview for Pirograph streams

// This version gives 4-up display... which it turns out I don't need. Hence not implemented.

// Runs on pi-top Ceeds to show a down-res version of the full image
// --Based on code by-- almost entirely lifted from Daniel Shiffman
// <http://www.shiffman.net>

// Receives JPEG image streams over UDP.
// Run using Processing's Present mode, with a black background set in Preferences.

// A Thread using receiving UDP to receive images

import java.awt.image.*; 
import javax.imageio.*;
import java.net.*;
import java.io.*;

PImage video;
ReceiverThread thread;

void setup() {
  size(960,540, P2D);
  video = createImage(960,540,RGB);
  thread = new ReceiverThread(video.width,video.height);
  thread.start();
}

 void draw() {
  if (thread.available()) {
    video = thread.getImage();
  }

  // Draw the image
  background(0);
  //imageMode(CENTER);
  image(video,0,0);
}
