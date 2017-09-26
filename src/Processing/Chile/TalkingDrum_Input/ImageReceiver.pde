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

class ImageReceiver {

  // Port we are receiving.
  int port = 9100; 
  DatagramSocket ds; 
  // A byte array to read into (max size of 65536, could be smaller)
  byte[] buffer = new byte[65536]; 

  PImage imgReceiver;
  boolean readingDone  = false;

  ImageReceiver(int imgWidth, int imgHeight) {
    try {
      ds = new DatagramSocket(port);
    } 
    catch (SocketException e) {
      e.printStackTrace();
    } 
    imgReceiver = createImage(imgWidth, imgHeight, RGB);
  }
  
  PImage getImage(){
    return imgReceiver;
  }

  void processInput() {
    // Draw the image
    if (!readingDone) {
      println("checking reading image");
      checkForImage();
      println("done reading image");
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
    imgReceiver.loadPixels();
    try {
      // Make a BufferedImage out of the incoming bytes
      BufferedImage img = ImageIO.read(bais);
      // Put the pixels into the video PImage
      img.getRGB(0, 0, imgReceiver.width, imgReceiver.height, imgReceiver.pixels, 0, imgReceiver.width);
      readingDone = true;
    } 
    catch (Exception e) {
      e.printStackTrace();
    }
    // Update the PImage pixels
    imgReceiver.updatePixels();
  }
}