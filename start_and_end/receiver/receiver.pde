
import processing.serial.*;
import http.requests.*;

Serial port;
byte n;
int i;
int cnt;

void setup() {
  fullScreen();

  port = new Serial(this, Serial.list()[Serial.list().length-1], 115200);

  n = (byte) 0;
  cnt = 0;
  
}

void draw() {

  if(port.available() > 0) {
    
    String incoming = port.readString();
    background(255,0,0);

    if(incoming != null && incoming.length() > 13){
      String b = incoming.substring(5,13);
      i = Integer.parseInt(b, 2);
      background(i);
      println(b + ":" + i);

      GetRequest get = new GetRequest("https://cityio.media.mit.edu/talkingdrums/image/send/" + cnt + "/" + i);
      get.send();
      cnt ++;

    }
  }


}
