boolean  local  = true;

//input  computer  address 195.168.1.6
//output computer  address 195.168.1.5

String remoteAddress = "195.168.1.5";
int    portReceived =  12010;
int    portSend    =   12020;

boolean waitForImage = false;

void setupOSC() {
  oscP5 = new OscP5(this, portReceived); 
  if (local == true) {
    myRemoteLocation = new NetAddress("127.0.0.1", portSend);
  } else {
    myRemoteLocation = new NetAddress(remoteAddress, portSend);
  }
}

void oscEvent(OscMessage theOscMessage) {
  /* check if theOscMessage has the address pattern we are looking for. */

  if (theOscMessage.checkAddrPattern("/name")==true) {
    receivedImageName = theOscMessage.get(0).stringValue();
    println(receivedImageName);
    waitForImage = true;
  }

}

//request image to the output program
void requestImage() {
  OscMessage myMessage = new OscMessage("/request");
  oscP5.send(myMessage, myRemoteLocation);
}


void readImage() {
  int id = 1;
  inImage = loadImage("../output_images/"+id+".png");
} 


//get the newest file for a directory.
private File getLatestFilefromDir(String dirPath) {
  File dir = new File(dirPath);
  File[] files = dir.listFiles();
  if (files == null || files.length == 0) {
    return null;
  }

  File lastModifiedFile = files[0];
  for (int i = 1; i < files.length; i++) {
    if (lastModifiedFile.lastModified() < files[i].lastModified()) {
      lastModifiedFile = files[i];
    }
  }
  return lastModifiedFile;
}