import processing.serial.*;
Serial myPort;  // Create object from Serial class


int BAUD_RATE = 115200;

void setup() {
  size(600, 600);

  setupGraphics();

  setupImage();

  pixImage = pixeleted(numberStepsX, numberStepsY);

  setupPort();
}


void draw() {

  //image(pixImage, inImage.width, 0);
  background(0);
  image(pg, 0, 0);
}