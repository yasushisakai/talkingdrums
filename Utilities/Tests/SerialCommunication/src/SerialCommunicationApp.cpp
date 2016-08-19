// Demonstrates using the ci::Serial class

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Serial.h"
#include "cinder/Text.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"

#include <boost/algorithm/string.hpp>

#include <sstream>
#include <vector>

using namespace ci;
using namespace ci::app;
using namespace std;

#define BUFSIZE 80
#define READ_INTERVAL 0.25

class SerialCommunicationApp : public App {
 public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
    void keyDown( KeyEvent event) override;
	void update() override;
	void draw() override;

	bool		mSendSerialMessage;
	SerialRef	mSerial;
    string		mLastString;
	
	gl::TextureRef	mTexture;
	
	double mLastRead, mLastUpdate;
private:
    int         pixelIndex;
    vector<uint8_t> rectangles;
};

void SerialCommunicationApp::setup()
{
    
    
    
	mCounter = 0;
	mLastRead = 0;
	mLastUpdate = 0;
	mSendSerialMessage = false;

	// print the devices
	for( const auto &dev : Serial::getDevices() )
		console() << "Device: " << dev.getName() << endl;

	try {
		Serial::Device dev = Serial::findDeviceByNameContains( "tty.usbserial" );
		mSerial = Serial::create( dev, 9600 );
	}
	catch( SerialExc &exc ) {
		CI_LOG_EXCEPTION( "coult not initialize the serial device", exc );
		exit( -1 );
	}
    
    pixelIndex = 0;
}

void SerialCommunicationApp::mouseDown( MouseEvent event )
{
}

void SerialCommunicationApp::keyDown(KeyEvent event)
{
    if(event.getChar()=='q'){
        quit();
    }
}

void SerialCommunicationApp::update()
{
//	console() << "Bytes available: " << mSerial->getNumBytesAvailable() << std::endl;
	
	double now = getElapsedSeconds();
	double deltaTime = now - mLastUpdate;
	mLastUpdate = now;
	mLastRead += deltaTime;
	
	if( mLastRead > READ_INTERVAL )	{
		mSendSerialMessage = true;
		mLastRead = 0.0;
	}

	if( mSendSerialMessage ) {
		
		try{
			// read until newline, to a maximum of BUFSIZE bytes
			mLastString = mSerial->readStringUntil( '\n', BUFSIZE );

		}
		catch( SerialTimeoutExc &exc ) {
			CI_LOG_EXCEPTION( "timeout", exc );
		}
		
		mSendSerialMessage = false;

        console() << mLastString << endl;
        
        
		TextLayout simple;
		simple.setFont( Font( "Arial Black", 24 ) );
        if(mLastString.find("L: r=")!= string::npos ){
            simple.setColor(Color(1.0,0,0));
            
            string string_bits = mLastString.substr(5);
            uint8_t value = 0;
            for (int i =0;i<8;i++){
                value += (string_bits[i]=='1') << (7-i);
            }
            
            rectangles.push_back(value);
            
            pixelIndex++;

        }else if(mLastString.find("start")!=string::npos){
                
        }
        
		//simple.setColor( Color( .7, .7, .2 ) );
		simple.addLine( mLastString );
		simple.setLeadingOffset( 0 );
		mTexture = gl::Texture::create( simple.render( true, false ) );

		mSerial->flush();
	}
}

void SerialCommunicationApp::draw()
{
	gl::clear();
	
	if( mTexture )
		gl::draw( mTexture, vec2( 10, 10 ) );
    
    for(int i=0;i<rectangles.size();++i){
        gl::color(Color8u(rectangles[i],rectangles[i],rectangles[i]));
        int x= i%15;
        int y = i/15;
        Rectf rect = Rectf(vec2(x*10,y*10),vec2((x+1)*10,(y+1)*10));
        gl::drawSolidRect(rect);
    }
    
    
    
    int x= pixelIndex%15;
    int y= pixelIndex/15;
    
    gl::color(1, 1, 1);
    Rectf rect = Rectf(vec2(x*10,y*10),vec2((x+1)*10,(y+1)*10));
    gl::drawStrokedRect(rect);
    
}

CINDER_APP( SerialCommunicationApp, RendererGl )
