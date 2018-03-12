import processing.video.*;

Capture cam;

void setup() {
  size(640, 480);
  
  String[] cameras = Capture.list();
  
  if (cameras.length == 0) {
    println("No available cameras.");
    exit();
  } else {
    println("Available cameras:");
    for (int i = 0; i < cameras.length; i++) {
      println(i, cameras[i]);
    }
  }
    
  cam = new Capture(this, cameras[253]);
  cam.start();
}

void draw() {
  if (cam.available()) {
    cam.read();
  }
  set(0, 0, cam);
}