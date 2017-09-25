import processing.serial.*;
import java.nio.file.Path;
import java.nio.file.Paths;

Serial myPort;  // Create object from Serial class

boolean initSerialPort = false;

//change this value to initialize the serial Id.
//default -1 which is Serial.list().length - 1
int serialId = 0;

boolean debug = false;

//get newest image from the file directory
boolean newImage = false;

//reading image information.
String IMAGE_NAME = "bb.png";//"music.png"; 
int BAUD_RATE = 115200;

int scalefraction = 0;

void setup() {
  size(1270, 800);

  setupGraphics();

  setupImage();

  pixImage = pixeleted(numberStepsX, numberStepsY);

  setupPort();
}


void draw() {
  background(50);


  if (debug) {
    drawImage();  
    image(pixImage, width/2.0, 0, width/2.0, height/2.0);
  } else {


    image(pixImage, 0, 0);

    noFill();
    stroke(0, 0, 255);
    rect(iterPixelX * numberStepsX, iterPixelY * numberStepsY, numberStepsX, numberStepsY);
  }
}