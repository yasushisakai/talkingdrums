import processing.serial.*;

int lf = 10;    // Linefeed in ASCII
int value;
String myString = null;
Serial myPort;  // The serial port

ArrayList points = new ArrayList();

void setup() {
  size(1000, 1000);
  background(0);
  // List all the available serial ports
  println(Serial.list());
  // I know that the first port in the serial list on my mac
  // is always my  Keyspan adaptor, so I open Serial.list()[0].
  // Open whatever port is the one you're using.
  myPort = new Serial(this, Serial.list()[1], 115200);
  myPort.clear();
  // Throw out the first reading, in case we started reading 
  // in the middle of a string from the sender.
  myString = myPort.readStringUntil(lf);
  myString = null;

  for (int i =0; i < 20; i++) {
    points.add(0);
  }
}

void draw() {
  while (myPort.available() > 0) {
    myString = myPort.readStringUntil(lf);
    if (myString != null) {
      myString = trim(myString);
      value = int(myString);


      points.remove(0);
      points.add(value);
    }
  }
  background(0);

  noFill();
  strokeWeight(5);
  stroke(255);
  beginShape();
  for (int i = 0; i < points.size(); i++) {
    int yVal = (Integer)points.get(i);

    //if (yVal > 150) {
     // println(yVal);
      vertex(i*30 + 200, 500 - yVal);
    //}
  }
  endShape();
}

void keyPressed() {
  if (key == 'a') {
    save(frameCount+".png");
  }
}