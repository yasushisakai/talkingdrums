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
int BAUD_RATE = 115200;

//change this value to initialize the serial Id.
//default -1 which is Serial.list().length - 1
int serialId = -1;

boolean debug = false;

//get newest image from the file directory
boolean newImage = false;

//reading image information.
String imageName = "unnamed.jpg";//"music.png"; 


int scalefraction = 0;


//received images sequeces
ImageReceiver  imageReceiver;
PImage newReceivedImage;
String receivedImageName = "";

String outputImageDir = "";

void setup() {
  fullScreen();

  outputImageDir = sketchPath("")+"../output_images";

  setupGraphics();

  setupImage();

  pixImage = pixeleted(numberStepsX, numberStepsY);
  pixImage = inImage;

  setupPort();

  setupOSC();

  imageReceiver = new ImageReceiver(inImage.width, inImage.height);

  flash();
}


void draw() {
  background(0);


  if (debug) {
    drawImage();  
    image(pixImage, width/2.0, 0, width/2.0, height/2.0);
  } else {
    image(pixImage, 0, 0);

    noFill();
    strokeWeight(5);
    stroke(255);
    rect(iterPixelX * numberStepsX, iterPixelY * numberStepsY, numberStepsX, numberStepsY);

    if (activateFlash) {
      image(flash, 0, 0, width, height);

      if (timeCounter > maxCounter) {
        activateFlash = false;
        timeCounter = 0;
      }
      timeCounter++;
    }
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