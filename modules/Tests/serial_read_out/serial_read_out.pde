
import processing.serial.*;


Serial port;
String[] serial_list = Serial.list();
boolean flag;
String inString;
int last_ms;
float prev_x, prev_y;

final float max_v = 3.3;
final float threshold = 2.0;

int ms_frame = 10000;

void setup() {
  size(640, 480);
  flag = false;
  int i;
  for (i=0; i<serial_list.length; i++) {
    // assuming you are using mac
    if (serial_list[i].startsWith("/dev/tty.usb")) {
      flag = !flag;
      println("connected to: "+serial_list[i]);
      break;
    }
  }

  if (!flag) {
    println("cannot find serial port");
    exit();
  } else {
    port = new Serial(this, serial_list[i], 9600);
    last_ms = millis();
    background(255);
    prev_x = 0.0;
    prev_y = height;
    rectMode(CORNERS);
  }
}

void draw() {
  fill(255, 5);
  noStroke();
  rect(0, 0, width, height);
  int current_ms = millis() - last_ms;
  if (current_ms < ms_frame) {
    if (port.available() > 0) inString = port.readStringUntil('\n');
    if (inString != null) {
      println(inString);
      float new_x = float(current_ms)/ms_frame * width;
      float new_y = (1.0-float(inString)/max_v) * height;
      float difference_ratio = abs(new_y-prev_y)/height;

      if (float(inString)>threshold) {
        noFill();
        stroke(255, 0, 0);
        strokeWeight(10);
        rect(0, 0, width, height);
      } else {
        strokeWeight(0.5);
      }

      stroke(255*difference_ratio, 0, 255*(1.0-difference_ratio));
      line(prev_x, prev_y, new_x, new_y);
      prev_x = new_x;
      prev_y = new_y;
    }
  } else {
    last_ms = millis();
    prev_x = 0.0;
    prev_y = height;
  }
}