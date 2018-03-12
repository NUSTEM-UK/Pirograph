// Basic thresholding to an RGBA destination
// This is doing the thresholding on RGB source pixels. Ugh.

PImage source;
PImage destination;
PImage img2;

void setup() {
  size(1650, 1080);
  background(0,255,0);
  source = loadImage("wishing_well.jpeg"); // LoadImage serves as constructor for PImage object. Hmm.
  destination = createImage(source.width, source.height, ARGB);
  img2 = createImage(200, 200, RGB); // make a new blank image
}

void draw() {
  float threshold = 50;
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