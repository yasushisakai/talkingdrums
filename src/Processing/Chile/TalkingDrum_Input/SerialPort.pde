String readPort;
byte [] mSendData;
int numBytes = 1;

void setupPort() {
  print("setting up Port");

  printArray(Serial.list());

  if (serialId == -1) {
    serialId = 1;//Serial.list().length -1;
  }

  String portName  = "";
  try {
    portName = Serial.list()[serialId];
    myPort = new Serial(this, portName, BAUD_RATE);
    readPort = "";
    initSerialPort = true;
    println("connected to: "+portName+" "+initSerialPort);

  }
  catch(Exception e) {
    initSerialPort = false;
    println("cannot connect to: "+portName);
  }

  if (initSerialPort) {

    int lf = 10;
    myPort.bufferUntil(lf);
  }



  mSendData = new byte[numBytes];
}

void serialEvent(Serial p) {
  if (initSerialPort) {
    readPort = p.readString();

    println(readPort);

    if (readPort.contains("s")) {
      println("starting sending"); 

      //get central
      int centralPixX = iterPixelX * numberStepsX + int(numberStepsX/2.0);
      int centralPixY = iterPixelY * numberStepsY + int(numberStepsY/2.0);

      color gray = pixImage.get(centralPixX, centralPixY);
      
      sendGrayColor(gray);
      updateIteration();
    }
  }
}


void updatePort() {
}

void sendGrayColor(color pixelColor) {
  int grayValue = int((red(pixelColor) + green(pixelColor) + blue(pixelColor))/3.0);

  mSendData[0] = (byte)grayValue;
  println(grayValue);

  byte b = (byte)grayValue;
  println(b);
  println(Integer.toBinaryString(grayValue));
  println(toBitString(grayValue, 8));


  myPort.write(mSendData);
}

String toBitString( int x, int bits ) {
  String bitString = Integer.toBinaryString(x);
  int size = bitString.length();
  StringBuilder sb = new StringBuilder( bits );
  if ( bits > size ) {
    for ( int i=0; i<bits-size; i++ )
      sb.append('0');
    sb.append( bitString );
  } else
    sb = sb.append( bitString.substring(size-bits, size) );

  return sb.toString();
}