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

 
  //stroke(255, 0, 0);


  //rect(xy[0]*50, xy[1]*50, 50, 50);

  if (port.available() > 0) serialIn = port.readStringUntil('\n');
  if (serialIn != null) {
    if (serialIn.startsWith("L: r=")) {
      // theres a log message!!
      String bits = serialIn.substring(serialIn.lastIndexOf('=')+1);
      println(bits);

      //println(bits.charAt(0)+" "+bits.charAt(1)+" "+bits.charAt(2)+" "+bits.charAt(3)+" "+bits.charAt(4)+" "+bits.charAt(5)+" "+bits.charAt(6)+" "+bits.charAt(7));
      // stroke();//+Byte.MIN_VALUE+1);
      stroke(200);
      fill(random(100), 200, 0);
       int[] xy = indexToXY(index);
      rect(xy[0]*50, xy[1]*50, 49, 49);
      index ++;
    }
  }
}

void keyPressed() {
  save("img_"+String.format("%06d", index)+".png");
}

int[] indexToXY(int index) {
  int[] result = new int [2];
  result[0] = index%5;
  result[1] = index/5;
  println(result[0]+" "+result[1]);
  return result;
}