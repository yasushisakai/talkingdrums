byte n;
SerialHandler s;

enum Mode {
  FETCH, 
    IDLE, 
    READY, 
    ERROR_SERVER, 
    ERROR_SERIAL,
}

Mode mode;
DisplayImg displayImg;

void setup() {
  size(1024, 600);
  background(0);
  smooth(16);

  displayImg = new DisplayImg();

  try {
    s = new SerialHandler(this);
    mode = Mode.FETCH;
  } 
  catch (Exception e) {
    mode = Mode.ERROR_SERIAL;
  }

  delay(2500);
}

void draw() {
  background(0);
  update(); // logic

  displayImg.drawFullScreen();

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
    }

    println("next byte is " + formatByte(n));

    if (isReady) {
      mode = Mode.READY;
    } else {
      mode = Mode.IDLE;
    }
    break;
  case ERROR_SERIAL :
    println("RECONNECTING TO SERIAL");
    delay(5000);
    try {
      s = new SerialHandler(this);
    } 
    catch (Exception e) {
      println(e);  
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
