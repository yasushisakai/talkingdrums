import processing.serial.*;

Serial port;
byte n;

void setup() {
  size(1068, 768);

  port = new Serial(this, Serial.list()[Serial.list().length-1], 115200);

  n = (byte) 0;
  
  // super slow 
  frameRate(1);
}

void draw() {
  int m = (n & 0xFF);
  background(m);

  String binStr = String.format("%8s", Integer.toBinaryString(n & 0xFF)).replace(' ','0');
  String message = "L: r=" + binStr;
  port.write(message);
  println(message);
  
  n--;
}
