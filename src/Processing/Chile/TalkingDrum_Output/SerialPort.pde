String readPort;
byte [] mSendData;
int numBytes = 1;

void setupPort() {
  print("setting up Port");

  printArray(Serial.list());

  if (serialId == -1) {
    serialId = Serial.list().length -1;
  }

  String portName  = "";
  try {
    portName = Serial.list()[serialId];
    myPort = new Serial(this, portName, BAUD_RATE);
    readPort = "";
    initSerialPort = true;
    println("connected to: "+portName);

    int lf = 10;
    myPort.bufferUntil(lf);
  }
  catch(Exception e) {
    initSerialPort = false;
    println("cannot connect to: "+portName);
  }



  mSendData = new byte[numBytes];
}

void serialEvent(Serial p) {
  readPort = p.readString();

  println(readPort);

  if (readPort.contains("L: r=")) {

    String[] arr = readPort.split("L: r=");

    if (arr.length >= 2) {
      println("got pixel"); 
      println(arr[1]);

      int intSerial = binaryToInteger(arr[1]);
      println(intSerial);

      offSreenRender(color(intSerial));

      updateIteration();
    }
  }
}

int binaryToInteger(String binary) {
  char[] numbers = binary.toCharArray();
  int result = 0;

  //orubt
  for (int i = 0; i < numbers.length; i++) {
    print(numbers[i]);
  }
  int numBit = 8;
  for (int i = numBit - 1; i>=0; i--)
    if (numbers[i]=='1') {
      result += Math.pow(2, (numBit - i - 1));
    }
  return result;
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