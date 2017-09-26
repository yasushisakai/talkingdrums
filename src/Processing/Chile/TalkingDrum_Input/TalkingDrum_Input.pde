/*
Talking Drums code for processing the input image to the talking drum system
 Thomas Sanchez Lengeling
 
 Yasushi Sekai
 Nicole L'Huillier
 */

import processing.serial.*;
import oscP5.*;
import netP5.*;

import java.nio.file.Path;
import java.nio.file.Paths;

//OSC Messages for transfering data from input to output
OscP5 oscP5;
NetAddress myRemoteLocation;

// Create object from Serial class
Serial myPort; 

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

ImageReceiver  imageReceiver;
PImage newReceivedImage;
String receivedImageName = "";

String outputImageDir = "";

void setup() {
  size(1270, 800);

  setupGraphics();

  setupImage();

  pixImage = pixeleted(numberStepsX, numberStepsY);

  setupPort();

  setupOSC();

  imageReceiver = new ImageReceiver(numPixelsX, numPixelsY);
  
  outputImageDir = sketchPath("")+"../output_images";
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

  //saving process, the recevied images from the output is locally saved.
  if (waitForImage) {
    imageReceiver.processInput();

    newReceivedImage = imageReceiver.getImage();
    newReceivedImage.save(outputImageDir+"/"+receivedImageName+".png");

    println("saved "+receivedImageName);
    waitForImage = false;
  }
}