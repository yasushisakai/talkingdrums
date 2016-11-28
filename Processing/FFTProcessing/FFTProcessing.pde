import processing.serial.*;
Serial myPort; 

int buffer[] = new int[64];
int bufferConter = 0;

void setup() {

  size(1024, 768);

  String portName = Serial.list()[0];
  myPort = new Serial(this, portName, 9600);
}

void draw() {
  if ( myPort.available() > 0) {  // If data is available,
    buffer[bufferConter] = myPort.read();         // read it and store it in val
    bufferConter++;
    if(bufferConter > 64){
     bufferConter = 0; 
    }
  }
  
  beginShape();
  for(int i = 0; i <64; i++){
  vertex(i*2, 300, i*2, 300 - buffer[i]);
  }
  endShape();
}