import processing.serial.*;
Serial myPort;  // The serial port

String myString = "";
int lf = 10;

Device device;

void setup() {
  size(500, 500);
  background(0);
  // List all the available serial ports
  for(int i = 0;i < Serial.list().length; i++){
   println(Serial.list()[i]); 
  }
  // I know that the first port in the serial list on my mac
  // is always my  Keyspan adaptor, so I open Serial.list()[0].
  // Open whatever port is the one you're using.
  
  // As of this RaspberryPi3, the index is 1
  myPort = new Serial(this, Serial.list()[1], 9600);
  myPort.clear();
  myString = myPort.readStringUntil(lf);
  myString = null;
  
  device = new Device(12, 10, 10);
}

void draw() {
  background(0);

  while (myPort.available() > 0) {
    myString = myPort.readStringUntil(lf);
    if (myString != null) {
      myString = trim(myString);
      println(myString);
    }
  }
  device.draw();
}

void mousePressed() {
  // myPort.write("012P25");
  // println("changed no.12 PWM value of 25");
  device.check(mouseX, mouseY);
}

void keyPressed() {

  if (key == '1') {
    myPort.write(0);
    println("sent 0");
  }

  if (key == '2') {
    myPort.write(1);
    println("sent Mode 1");
  }

  if (key == '3') {
    myPort.write(2);
    println("sent Mode 2");
  }

  if (key == '4') {
    myPort.write(3);
    println("sent Mode 3");
  }

  if (key == '5') {
    myPort.write(4);
    println("sent Mode 4");
  }

  if (key == '6') {
    myPort.write(5);
    println("sent Mode 5");
  }

  if (key == '7') {
    myPort.write(6);
    println("sent Mode 6");
  }

  if (key == '8') {
    myPort.write(7);
    println("sent Mode 7");
  }

  if (key == '9') {
    myPort.write(8);
    println("sent Mode 8");
  }
  
   if (key == 'a') {
    myPort.write(9);
    println("sent Mode 9");
  }
  
   if (key == 's') {
    myPort.write(10);
    println("sent Mode 10");
  }
}