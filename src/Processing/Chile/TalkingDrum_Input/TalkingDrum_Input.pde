import processing.serial.*;
Serial myPort;  // Create object from Serial class

boolean initSerialPort = false;

//change this value to initialize the serial Id.
//default -1 which is Serial.list().length - 1
int serialId = -1;


boolean debug = false;


//reading image information.
String IMAGE_NAME = "music.png"; 
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
  background(0);


  if (debug) {
    drawImage();  
    image(pixImage, width/2.0, 0,  width/2.0, height/2.0);
  }else{
     image(pixImage, 0, 0);
  }
  
}