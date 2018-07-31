import processing.serial.*;
import http.requests.*;

Serial port;
byte n;
int i;
int cnt;

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
  cnt = 0;
}

void draw() {
  if(port.available() > 0 && mode != Mode.SERIAL_ERROR) {
    String incoming = port.readString();
    background(255,0,0);

    if(incoming != null && incoming.length() > 13){
      String b = incoming.substring(5,13);
      i = Integer.parseInt(b, 2);
      background(i);
      println(b + ":" + i);
      
      try {
        GetRequest get = new GetRequest("https://cityio.media.mit.edu/talkingdrums/image/send/" + i);
        get.send();
        cnt ++;
        mode = Mode.RUNNING;
      } catch (Exception e) {
        println("error sending to server"); 
        mode = Mode.SERVER_STOPPED;
      }
    }

    if(mode == Mode.RUNNING) {
      pushStyle();
      fill(255, 0, 0);
      text("received: " + i, 15, 15 );
      popStyle();
    } else {
      pushStyle();  
      stroke(255, 0, 0);
      noFill();
      rect(0, 0, width, height);
      println("error code: " + mode);
      popStyle();
    }
        
  }


}
