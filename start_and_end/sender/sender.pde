import processing.serial.*;
import http.requests.*;

Serial port;
byte n;
boolean willSend = false;
boolean isSerial= true;

void setup() {
  size(1068, 768);
  try {
    port = new Serial(this, Serial.list()[Serial.list().length-1], 115200);
  } catch (Exception e) {
    isSerial = false;
  }
  n = (byte) 0;
  
  // super slow 
  frameRate(0.1);
}

void draw() {
  int m = (n & 0xFF);
  background(m);

  try {
    GetRequest get = new GetRequest(
      "https://cityio.media.mit.edu/talkingdrums/image/receive");
    get.send();
    JSONObject obj = parseJSONObject(get.getContent());
    println(obj.get("cnt"));
    if(!willSend){
      willSend = true;
      println("**sending enabled**");
    }
    n = (byte)obj.getInt("cnt");
  } catch (Exception e){
    println("server not sending bytes");
    willSend = false; 
  }

  if(willSend) sendMessage(n);
}

void sendMessage(byte d) {
  String binStr = String.format("%8s", Integer.toBinaryString(d & 0xFF)).replace(' ','0');
  String message = "L: r=" + binStr;

  port.write(message);
  println("Sent message: \""+ message + "\"");
}
