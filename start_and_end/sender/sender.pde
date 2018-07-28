
import processing.serial.*;
import http.requests.*;

Serial port;
byte n;

void setup() {
  size(1068, 768);
  // port = new Serial(this, Serial.list()[Serial.list().length-1], 115200);
  n = (byte) 0;
  
  // super slow 
  frameRate(1);
}

void draw() {
  int m = (n & 0xFF);
  background(m);

  String binStr = String.format("%8s", Integer.toBinaryString(n & 0xFF)).replace(' ','0');
  String message = "L: r=" + binStr;
  // port.write(message);
  println(message);
  
  // n--;

  GetRequest get = new GetRequest(
  "https://cityio.media.mit.edu/talkingdrums/image/receive"
  );
  get.send();
  JSONObject obj = parseJSONObject(get.getContent());
  println(obj.get("cnt"));
  n = (byte)obj.getInt("cnt");
}
