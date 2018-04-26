// Daniel Shiffman
// <http://www.shiffman.net>

// A Thread using receiving UDP to receive images

import java.awt.image.*; 
import javax.imageio.*;
import java.net.*;
import java.io.*;

PImage video;
ReceiverThread thread;

void setup() {
  size(640,360, P2D);
  video = createImage(640,360,RGB);
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
