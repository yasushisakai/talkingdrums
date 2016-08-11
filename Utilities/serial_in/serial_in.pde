import processing.serial.*;


Serial port;
String serialIn;
int last_ms;
int index;

void setup() {
  size(640, 480);

  String[] serial_list = Serial.list();
  boolean flag;

  flag = false;
  int i;
  for (i=0; i<serial_list.length; i++) {
    // assuming you are using mac
    if (serial_list[i].startsWith("/dev/tty.usb")) {
      port = new Serial(this, serial_list[i], 9600);
      last_ms = millis();
      background(255);
      rectMode(CORNERS);
      flag = !flag;
      println("connected to: "+serial_list[i]);
      break;
    }
  }

  if (!flag) {
    println("could not find port");
    exit();
  }

  index = 0;
  background(0);
}

void draw() {

  int[] xy = indexToXY(index);
  stroke(255, 0, 0);
  point(xy[0], xy[1]);

  if (port.available() > 0) serialIn = port.readStringUntil('\n');

  if (serialIn.startsWith("L: r=")) {
    // theres a log message!!
    String bits = serialIn.substring(serialIn.lastIndexOf('=')+1);
    stroke(unbinary(bits)+Byte.MIN_VALUE+1);
    point(xy[0],xy[1]); // plot! plot!
    index ++;
  }
  
}

void keyPressed(){
  save("img_"+String.format("%06d", index)+".png");
}

int[] indexToXY(int index) {
  int[] result = new int [2];
  result[0] = index/width;
  result[1] = index%width;
  return result;
}