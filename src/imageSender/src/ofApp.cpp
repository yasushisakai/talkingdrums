#include "ofApp.h"

ofImage image;
ofPixels pixels;
ofPoint start;
ofSerial serial;
int multi = 7;
int interval = 10; //seconds
double percent = 0.0;
int cursor = 0;
uint8_t value = 0;
bool isIncr = false;
bool isReady = false;

auto lastIncr = std::chrono::system_clock::now();

//--------------------------------------------------------------
void ofApp::setup(){

  // serial.listDevices();
  bool isReady = serial.setup(0, 115200);

  if(isReady) {
    ofLog(OF_LOG_NOTICE) << "serial ready" ;
  }

  image.load("image.gif");
  image.getTexture().setTextureMinMagFilter(GL_NEAREST,GL_NEAREST);
  pixels = image.getPixels();
  
  // center the big image
  start.x = (ofGetWindowWidth() - (image.getWidth() * multi)) * 0.5;
  start.y = (ofGetWindowHeight() - (image.getHeight() * multi)) * 0.5;

  // ofLog(OF_LOG_NOTICE) << "x: " << ofGetWindowWidth() << "y: " << ofGetWindowHeight() << endl;

  incrementCursor(cursor, value);

  // json example
  /*
  using json = nlohmann::json;
  
  auto re = ofLoadURL("http://biteater.media.mit.edu/image/send/1/255/");
  auto j = json::parse(re.data);
  ofLog(OF_LOG_NOTICE) << j["key"] << endl;
  */

}

//--------------------------------------------------------------
void ofApp::incrementCursor(int &c, uint8_t &v){

  int x = c % (int)(image.getWidth());
  int y = c / (int)(image.getWidth());

  ofColor p = pixels.getColor(x, y);

  v = p.r;

  if(!serial.writeByte(v)){
    ofLog(OF_LOG_NOTICE) << "could not send byte";
  } else {
    ofLog(OF_LOG_NOTICE) << "wrote on serial";
  }

  c++;
}

//--------------------------------------------------------------
void ofApp::update(){

  // check for the 's'
  if(serial.available() > 1) {
    int result = serial.readByte();

    ofLog(OF_LOG_NOTICE) << "result " << result;

    if (result == 115) isReady = true; 
  }

  auto now = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed = now - lastIncr;
  
  if(elapsed.count() > interval) {
    lastIncr = now;
    isIncr = true;
    
    if (isReady) incrementCursor(cursor, value); 
  }else {
    percent = elapsed.count() / (double)interval;
  }

}

//--------------------------------------------------------------
void ofApp::draw(){
  ofBackground(0);
  ofSetColor(ofColor::white);

  int x = start.x;
  int y = start.y - multi - 3;
  
  ofFill();
  ofDrawRectangle(x, y, percent * image.getWidth() * multi, 3);


  image.draw(start.x, start.y, image.getWidth() * multi, image.getHeight() * multi);
  x = start.x + multi * (cursor % (int)image.getWidth());
  y = start.y + multi * (cursor / (int)image.getWidth());

  ofSetColor(ofColor::red);
  ofNoFill();
  ofDrawRectangle(x, y, multi, multi);

  ofSetColor(value);
  ofFill();
  x = start.x;
  y = start.y + image.getHeight() * multi + 10;
  ofDrawRectangle(x, y, 100, 100);

  if(isIncr && isReady) {
    ofClear(ofColor::white);
    isReady = false;
    isIncr = false;
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
