import processing.video.*;

import java.net.DatagramSocket;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.io.ByteArrayInputStream;

import java.net.SocketException;

import java.io.OutputStream.*;
import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;


import javax.imageio.*;
import java.awt.image.*; 


// Port we are receiving.
int port = 9100; 
DatagramSocket ds; 
// A byte array to read into (max size of 65536, could be smaller)
byte[] buffer = new byte[65536]; 

PImage video;
boolean readingDone  = false;

void setup() {
  size(1280, 720);
  try {
    ds = new DatagramSocket(port);
  } 
  catch (SocketException e) {
    e.printStackTrace();
  } 
  video = createImage(1280, 720, RGB);
}

void draw() {
  // Draw the image
  if (!readingDone) {
    println("checking reading image");
    checkForImage();
    println("done reading image");
  }

  background(0);
  imageMode(CENTER);
  image(video, width/2, height/2);

  if (keyPressed == true) {
    if (key == 'a') {
      // checkForImage() is blocking, stay tuned for threaded example!
    }
  }
}


void checkForImage() {
  DatagramPacket p = new DatagramPacket(buffer, buffer.length); 
  try {
    ds.receive(p);
  } 
  catch (IOException e) {
    e.printStackTrace();
  } 
  byte[] data = p.getData();

  println("Received datagram with " + data.length + " bytes." );

  // Read incoming data into a ByteArrayInputStream
  ByteArrayInputStream bais = new ByteArrayInputStream( data );

  // We need to unpack JPG and put it in the PImage video
  video.loadPixels();
  try {
    // Make a BufferedImage out of the incoming bytes
    BufferedImage img = ImageIO.read(bais);
    // Put the pixels into the video PImage
    img.getRGB(0, 0, video.width, video.height, video.pixels, 0, video.width);
    readingDone = true;
  } 
  catch (Exception e) {
    e.printStackTrace();
  }
  // Update the PImage pixels
  video.updatePixels();
}