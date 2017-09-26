
///save all the incomming pixels locally

//if closed the app, save the image generated.

boolean  local  = true;
String remoteAddress = "195.168.1.6";
int    portReceived =  12020;
int    portSend    =   12010;

boolean sendImage = false;

void setupOSC() {
  oscP5 = new OscP5(this, portReceived); 
  if (local == true) {
    myRemoteLocation = new NetAddress("127.0.0.1", portSend);
  } else {
    myRemoteLocation = new NetAddress(remoteAddress, portSend);
  }
}

void transferImage() {

  if (sendImage) {

    int s = second();  // Values from 0 - 59
    int mi = minute(); 
    int d = day();    // Values from 1 - 31
    int m = month();  // Values from 1 - 12
    int y = year();

    String timeString = "td_"+y+"_"+m+"_"+d+"_"+mi+"_"+s;

    OscMessage myMessage = new OscMessage("/name");
    myMessage.add(timeString);
    oscP5.send(myMessage, myRemoteLocation); 
    println("sent "+timeString);
    //dont use this...easy hack


    delay(1000);

    //wait 1 seconds
    //now send Image
    
   imageSender.broadcast(pixImage);
    
    
    sendImage = false;
  }
}

void oscEvent(OscMessage theOscMessage) {
  //request Image from input

  if (theOscMessage.checkAddrPattern("/request")==true) {
    sendImage = true;
    println("activate sending Image");
  }
}


void saveImage() {

  int s = second();  // Values from 0 - 59
  int mi = minute(); 
  int d = day();    // Values from 1 - 31
  int m = month();  // Values from 1 - 12
  int y = year();

  String timeString = "td_"+y+"_"+m+"_"+d+"_"+mi+"_"+s;

  println("saved "+timeString);

  pg.save("../output_images/"+timeString+".png");
}