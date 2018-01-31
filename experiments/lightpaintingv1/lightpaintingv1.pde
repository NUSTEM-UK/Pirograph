import processing.opengl.*;
import com.jogamp.opengl.GL;
import com.jogamp.opengl.GL2ES2;
//import javax.media.opengl.*;

float currentCursorX;
float currentCursorY;
float prevX;
float prevY;
int numLayers = 4;

PJOGL pgl;
GL2ES2 gl;

float amountPainted = 0;
float currentSpeed;

int quality = 1; // 1 = normal, 2 = half quality

float[] amplitude = new float[numLayers];
float[] radius = new float[numLayers];
int[] lineStroke = new int[numLayers];

boolean isPainting = false;

void setup() {
    size(640, 480, OPENGL);
    frameRate(60);
    clearWindow();

    amplitude[0] = 100;
    amplitude[1] = 83;
    amplitude[2] = 66;
    amplitude[3] = 43;

    radius[0] = 0;
    radius[1] = -3;
    radius[2] = 6;
    radius[3] = -8;
    
    lineStroke[0] = 3;
    lineStroke[1] = 3;
    lineStroke[2] = 2;
    lineStroke[3] = 1;
}

void draw() {
    
    if (isPainting) {
    // Cursor position attenuation for smoothing
    currentCursorX -= (currentCursorX - (mouseX/quality)) / 2;
    currentCursorY -= (currentCursorY - (mouseY/quality)) / 2;

   
//    currentCursorX = mouseX/quality;
//    currentCursorY = mouseY/quality;

    drawLines(prevX, prevY, currentCursorX, currentCursorY);

    prevX = currentCursorX;
    prevY = currentCursorY;
    
    }
    
    if (mousePressed) {
    if (!isPainting) startPainting();
    } else {
    if (isPainting) stopPainting();
    }
}

void clearWindow() {
    background(0);
}

void startPainting() {
    currentSpeed = 0;
    clearWindow();
    isPainting = true;
    prevX = currentCursorX = mouseX/quality;
    prevY = currentCursorY = mouseY/quality;
}

void stopPainting() {
    isPainting = false;
}

void drawLines(float x1, float y1, float x2, float y2) {
    // Draw all lines with variations
    int i;
    float xDist = x2 - x1;
    float yDist = y2 - y1;
    float lineLength = sqrt(xDist * xDist + yDist * yDist);
    float newAmountPainted = amountPainted + lineLength;
    
    float newSpeed = 1 - (lineLength / 80);
    if (newSpeed < 0) newSpeed = 0;
    
    float cs = 0.1 + currentSpeed * 1.4;
    float ns = 0.1 + newSpeed * 1.5;

    pgl = (PJOGL) beginPGL();
    gl = pgl.gl.getGL2ES2();
    gl.glDisable(GL.GL_DEPTH_TEST);
    gl.glEnable(GL.GL_BLEND);
    gl.glBlendFunc(GL.GL_SRC_ALPHA,GL.GL_ONE);
    endPGL();

    // Erase buffer
    //background(0);

    // Draw the lines
    for (i = 0; i < numLayers; i++) {
    drawLine(x1, y1, amountPainted/amplitude[i], radius[i] * cs, x2, y2, newAmountPainted/amplitude[i], radius[i] * ns, lineStroke[i] * ((cs+ns)/2));
    }

    amountPainted = newAmountPainted;
    currentSpeed = newSpeed;
}

void drawLine(float x1, float y1, float amp1, float rad1, float x2, float y2, float amp2, float rad2, float __strokeWeight) {
    float ox1 = x1 + sin(amp1) * rad1;
    float oy1 = y1 + cos(amp1) * rad1;
    float ox2 = x2 + sin(amp2) * rad2;
    float oy2 = y2 + cos(amp2) * rad2;
    stroke(#885544);
    smooth();
    strokeWeight(__strokeWeight);
//  strokeCap(ROUND);
    line(ox1, oy1, ox2, oy2);
}