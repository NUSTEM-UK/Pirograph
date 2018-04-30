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

PImage video0;
PImage video1;
PImage video2;
PImage video3;
ReceiverThread thread0;
ReceiverThread thread1;
ReceiverThread thread2;
ReceiverThread thread3;

void setup() {
  // size(1920, 1080, P2D);
  // Go fullscreen on screen 2; should be possible to shift between dekstops.
  fullScreen(P2D, 1);
  background(0);
  video0 = createImage(960,540,RGB);
  video1 = createImage(960,540,RGB);
  video2 = createImage(960,540,RGB);
  video3 = createImage(960,540,RGB);
  thread0 = new ReceiverThread(video0.width,video0.height, 9100);
  thread1 = new ReceiverThread(video1.width,video0.height, 9101);
  thread2 = new ReceiverThread(video3.width,video0.height, 9102);
  thread3 = new ReceiverThread(video2.width,video0.height, 9103);
  thread0.start();
  thread1.start();
  thread2.start();
  thread3.start();
}

 void draw() {
  if (thread0.available()) {
    video0 = thread0.getImage();
    image(video0,0,0,960,540);
  }
  if (thread1.available()) {
    video1 = thread1.getImage();
    image(video1,960,0,960,540);
  }
  if (thread2.available()) {
    video2 = thread2.getImage();
    image(video2,0,540,960,540);
  }
  if (thread3.available()) {
    video3 = thread3.getImage();
    image(video3,960,540,960,540);
  }
}
