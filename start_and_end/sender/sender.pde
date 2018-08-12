byte n;
SerialHandler s;

PImage original;
PImage current;

enum Mode{
  FETCH,
  IDLE,
  READY,
  ERROR_SERVER,
  ERROR_SERIAL, 
}

Mode mode;
DisplayImg displayImg;

void setup() {
  //size(1024, 600);
  fullScreen();
  background(0);
  smooth(16);
  noCursor();

  displayImg = new DisplayImg();

  try {
    s = new SerialHandler(this);
    mode = Mode.FETCH;
  } 
  catch (Exception e) {
    mode = Mode.ERROR_SERIAL;
  }

  original = loadImageWithFallback("original");
  current = loadImageWithFallback("current");

}

void draw() {
  background(0);
  update(); // logic

  displayImg.drawFullScreen();
 }

// conditional logic
void update() {
  boolean isReady = false;
  if (mode != Mode.ERROR_SERIAL) { 
    try {
      if (s.checkReady()) {
        isReady = true;
      }
    } 
    catch (Exception e) {
      println(e);
    }
  }
  switch(mode) {
  case ERROR_SERVER:
    println("RECONNECTING to SERVER in 5sec");
    delay(5000);
  case FETCH:
    try {
      n = requestPixel();
    } 
    catch (Exception e) {
      println("SERVER ERROR: "+ e);
      mode = Mode.ERROR_SERVER;
      break;
    case READY:
      println("SENDING: " + formatByte(n));
      println();
      s.sendByte(n);
      mode = Mode.FETCH;
      isReady = false;
      current = loadImageWithFallback("current");
      break;
    case IDLE:
      if(isReady){
        mode = Mode.READY;
      }
      break;
    default:
      break;
    }
    mode = Mode.FETCH;
    break;
  case READY:
    println("SENDING: " + formatByte(n));
    println();
    s.sendByte(n);
    mode = Mode.FETCH;
    isReady = false;
    break;
  case IDLE:
    if (isReady) {
      mode = Mode.READY;
    }
    break;
  default:
    break;
  }
}

void keyPressed() {
  if (key == 'f') {
    fullScreen();
  }
}

//display error mode
void displayMode(Mode mode) {
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
  case FETCH:
    stroke(0, 0, 255);
    noFill();
    rect(0, 0, width, height);
    break;
  case READY:
    noStroke();
    fill(255);
    rect(0, 0, width, height);
    break;
  case IDLE:
    stroke(255);
    fill(int(n));
    rect(10, 10, 10, 10);
  default:
    break;
  }
  popStyle();
}
