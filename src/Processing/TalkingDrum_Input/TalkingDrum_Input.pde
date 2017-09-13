import processing.serial.*;
Serial myPort;  // Create object from Serial class

String IMAGE_NAME = "music.png"; 
int BAUD_RATE = 115200;

int scalefraction = 0;

void setup() {
  size(1920, 900);
  
  setupGraphics();
  
  setupImage();
  
  pixImage = pixeleted(numberStepsX, numberStepsY);
  
  setupPort();
}


void draw() {
  background(255);
  
  drawImage();
  
  image(pixImage, inImage.width, 0);
  
}