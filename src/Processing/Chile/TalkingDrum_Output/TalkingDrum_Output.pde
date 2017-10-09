/*
Talking Drums code for processing the output image an transfering back to the input program
 
 Thomas Sanchez Lengeling
 
 Yasushi Sekai
 Nicole L'Huillier
 */
import processing.serial.*;
import oscP5.*;
import netP5.*;

//OSC Messages for transfering data from input to output
OscP5 oscP5;
NetAddress myRemoteLocation;

//serial communication
Serial myPort;  // Create object from Serial class


boolean initSerialPort = false;

//change this value to initialize the serial Id.
//default -1 which is Serial.list().length - 1

int serialId = -1;

int BAUD_RATE = 115200;

ImageSender imageSender;

void setup() {
  //fullScreen();

  size(700, 700);


  setupGraphics();

  setupImage();

  pixImage = pixeleted(numberStepsX, numberStepsY);
  pixImage = inImage;

  setupPort();

  setupOSC();

  imageSender = new ImageSender();
  
  flash();
}


void draw() {
  //background(0);

  //image(pixImage, 0, 0);//inImage.width, 0);
  //background(0);
  image(pg, 0, 0, width, height);

  if (activateFlash) {
    image(flash, 0, 0, width, height);

    if (timeCounter > maxCounter) {
      activateFlash = false;
      timeCounter = 0;
    }
    timeCounter++;
  }
  
}

void keyPressed() {
  if (key == 'a') {
    saveImage();
  }

  if (key == '1') {
    transferImage();
  }
  
  if(key == '2'){
   activateFlash = true; 
  }

  if (key == '3') {

    for (int i  =0; i < 1000; i++) {
      offSreenRender(color(0, random(100), 0));
      updateIteration();
    }
  }
}


void exit() {
  println("exiting");
  sendImage = true;
  transferImage();
}