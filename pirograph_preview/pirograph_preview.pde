// Receiver / preview for Pirograph streams
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

int THISPORT; // to which channel are we subscribing?

int[] clientPorts = {9100, 9101, 9102, 9103};

void setup() {
  size(960,540, P2D);

  // We should have a command-line argument to tell us which channel to receive
  if (args != NULL){
    THISPORT = int(args[1]);
  } else {
    // default to port A
    println("args == null");
    THISPORT = 0'
  }

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
