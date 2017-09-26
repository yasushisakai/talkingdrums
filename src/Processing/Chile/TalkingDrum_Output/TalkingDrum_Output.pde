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
  size(600, 600);

  setupGraphics();

  setupImage();

  pixImage = pixeleted(numberStepsX, numberStepsY);
  
  setupPort();
  
  setupOSC();
  
  imageSender = new ImageSender();
}


void draw() {

  image(pixImage, 0, 0);//inImage.width, 0);
  //background(0);
  //image(pg, 0, 0);
  
}

void keyPressed() {
  if (key == 'a') {
    saveImage();
  }
  
  if(key == '1'){
   transferImage(); 
  }
  

}


void exit() {
  println("exiting");
 
  saveImage();
}