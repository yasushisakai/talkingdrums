import processing.serial.*;
import http.requests.*;

Serial port;
byte n;


enum Mode {
  INITIAL,
  RUNNING,
  SERVER_STOPPED,
  SERIAL_ERROR, 
}

Mode mode = Mode.INITIAL;

void setup() {
  fullScreen();
  try {
    port = new Serial(this, Serial.list()[Serial.list().length-1], 115200);
  } catch (Exception e) {
    mode = Mode.SERIAL_ERROR;
  }

  n = (byte) 0;
  
  // super slow 
  frameRate(0.055555); // approx. 18sec / frame
}

void draw() {
  int m = (n & 0xFF);
  background(m);

  try {
    GetRequest get = new GetRequest(
      "https://cityio.media.mit.edu/talkingdrums/image/get/next");
    get.send();
    JSONObject obj = parseJSONObject(get.getContent());
    n = (byte) obj.getInt("value");

    if(mode != Mode.SERIAL_ERROR) {
      mode = Mode.RUNNING; 
    }

  } catch (Exception e){
    // might be a json parseing error too
    println("server not sending bytes, Exception: " + e);
    mode = Mode.SERVER_STOPPED;
  }

  if (mode == Mode.RUNNING) {
    pushStyle();
    fill(255, 0, 0);
    text("sending: " + n, 15, 15);
    popStyle();
    sendMessage(n);
  }

  if(mode != Mode.RUNNING) {
    pushStyle();
    stroke(255, 0, 0);
    rect(0, 0, width, height);
    popStyle();
    println("error code: " + mode);
  }
}

void sendMessage(byte d) {
  // String binStr = String.format("%8s", Integer.toBinaryString(d & 0xFF)).replace(' ','0');
  // String message = "L: r=" + binStr;

  // port.write(message);
  //
  port.write(d); // because the communication protocol is different in and out
  println("Sent message: \""+ d + "\"");
}
