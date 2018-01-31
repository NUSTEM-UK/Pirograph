import processing.video.*;
 
Capture cam;
 
PImage[] buffer;
 
int w = 640;
int h = 460;
int nFrames = 10;
 
int iWrite = 0, iRead = 1;
 
void setup(){
  size(640,480);
  cam = new Capture(this, w, h);
  cam.start();
  buffer = new PImage[nFrames];
}
 
void draw() {
  if(cam.available()) {
    cam.read();
    image(cam,0,0);
 
    buffer[iWrite] = cam.get();
    if(buffer[iRead] != null){
      tint(255,120);
      image(buffer[iRead], 0, 0);
    }
 
    iWrite = (iWrite + 1) % nFrames;
    iRead  = (iRead  + 1) % nFrames;
    //iWrite++;
    //iRead++;
    //if(iRead >= nFrames-1){
    //  iRead = 0;
    //}
    //if(iWrite >= nFrames-1){
    //  iWrite = 0;
    //}
  }       
}