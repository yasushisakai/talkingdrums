byte n;
SerialHandler s;

enum Mode{
  IDLE,
  PUSH,
  ERROR_SERVER,
  ERROR_SERIAL, 
}

Mode mode;

void setup() {
  size(1024, 600);

  try{
    s = new SerialHandler(this);
    mode = Mode.IDLE;
  } catch (Exception e) {
    mode = Mode.ERROR_SERIAL;
  }
  
  delay(2500);
  
  n = 0;
}

void draw() {
  background(0);
  update(); // logic
  
  // rendering
  pushStyle();
  switch (mode) {
    case ERROR_SERVER:
    case ERROR_SERIAL:
      fill(255, 0, 0);
      noStroke();
      rect(0, 0, width, height);
      fill(0);
      text("error", 5, 15);
    break;
    case PUSH:
      noStroke();
      fill(255);
      rect(0, 0, width, height);
      break;
    case IDLE:
      stroke(255);
      fill(int(n));
      rect(10,10,10,10);
    default:
    break;
  }
  popStyle();
}

// conditional logic
void update() {

  switch(mode){
    case IDLE:
      try {
        n = s.getByte();
      } catch (Exception e) {
        println("MESSAGE ERROR: "+ e);
        break;
      }
      mode = Mode.PUSH;
      break;
    case ERROR_SERIAL :
      println("RECONNECTING TO SERIAL");
      delay(5000);
      try {
        s = new SerialHandler(this);
      } catch (Exception e) {
        println(e);  
        break;
      }
      mode = Mode.IDLE;
      break;
    case ERROR_SERVER:
      println("RECONNECTING to SERVER in 5sec");
      delay(5000);
    case PUSH:
      try{
        sendPixel(n);
      } catch (Exception e){
        println("Error pushing pixel: "+ e);
        mode = Mode.ERROR_SERVER;
        break;
      }
      println("PUSHED: " + formatByte(n));
      println();
      mode = Mode.IDLE;
      break;
    default:
      break;
  }
}
