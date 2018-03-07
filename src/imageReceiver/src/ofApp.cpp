#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){

  // initiate the image
  image.allocate(imageWidth, imageHeight, OF_IMAGE_GRAYSCALE);
  image.getTexture().setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);
  // do we want to wash it?
  // serial.listDevices();
  bool isReady = serial.setup(0, 115200);

  if(isReady) {
    ofLog(OF_LOG_NOTICE) << "serial ready" ;
  }

  //center the big image
  start.x = (ofGetWindowWidth() - imageWidth * multi) * 0.5;
  start.y = (ofGetWindowHeight() - imageHeight * multi) * 0.5;

}

//--------------------------------------------------------------
void ofApp::update(){

  // check for 'L: r=1111111'

  if(serial.available() > 12) {
    int result = serial.readBytes(bytes, serial.available());
    
    switch (result) {
      case OF_SERIAL_ERROR:
        ofLog(OF_LOG_ERROR, "error reading serial");
        break;

      case OF_SERIAL_NO_DATA:
        ofLog(OF_LOG_NOTICE, "no data");
        break;

      default:
        string message(reinterpret_cast<char*>(bytes));
        ofLog(OF_LOG_NOTICE) << "bytes: " << message << endl;
        if(message.compare(0, checkHead.length(), checkHead) == 0){
          // match
          string vb(message.substr(checkHead.length(), 7));
          bitset<7> v (vb);

          value = static_cast<uint8_t>(v.to_ulong());
          image.setColor(cursor, value);
          
          stringstream url;
          url << base_url;
          url << cursor;
          url << "/";
          url << value;
          url << "/";

          // TODO: better if async
          ofHttpResponse res = ofLoadURL(url.str());

          if(res.error != "") {
            ofLog(OF_LOG_ERROR, res.error);
          }

          cursor ++;
          hasReceived = true;
          serial.flush();
        }
    }
  }
}

//--------------------------------------------------------------
void ofApp::draw(){
  ofBackground(0);
  ofSetColor(ofColor::white);

  image.draw(start.x, start.y, image.getWidth() * multi, image.getHeight() * multi);

  int x = start.x + multi * (cursor % (int)image.getWidth());
  int y = start.y + multi * (cursor / (int)image.getWidth());

  ofSetColor(ofColor::red);
  ofNoFill();
  ofDrawRectangle(x, y, multi, multi);

  ofSetColor(value);
  ofFill();
  x = start.x;
  y = start.y + image.getHeight() * multi + 10;
  ofDrawRectangle(x, y, 100, 100);

  if(hasReceived) {
    ofClear(ofColor::white);
    hasReceived = false;
  }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
