#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/Serial.h"
#include "cinder/Log.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define INTERVAL 0.25

class cinderSandboxApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
    void mouseMove( MouseEvent event ) override;
	void update() override;
	void draw() override;
  private:
    SerialRef mSerial;
    double mLastRead;
    bool mReadBuffer;
    
};

void cinderSandboxApp::setup()
{
    setWindowSize(200,200);
    
    
    try{
        Serial::Device dev = Serial::findDeviceByNameContains("tty.usb");
        mSerial = Serial::create(dev,9600);
    }catch(SerialExc &exc){
        CI_LOG_EXCEPTION("no available Serial device",exc);
        exit(-1);
    }
    
    char start_sign;
    while(start_sign != '*'){
        start_sign = (char)mSerial->readByte();
    }
    
    CI_LOG_D("recieved data: " << start_sign );
    
    CI_LOG_D("start to read");
    mSerial->writeByte('r');
    
    mLastRead = getElapsedSeconds();
    mReadBuffer = false;

    
}

void cinderSandboxApp::mouseDown( MouseEvent event )
{
}

void cinderSandboxApp::mouseMove(MouseEvent event){
}

void cinderSandboxApp::update()
{
    double currentTime = getElapsedSeconds();
    if(currentTime-mLastRead > INTERVAL){
        mLastRead = currentTime;
        mReadBuffer = true;
    }
    
    if(mReadBuffer){
        string buffer;
        
        try{
        buffer = mSerial->readStringUntil('\n',80);
        }catch(SerialTimeoutExc exc){
            CI_LOG_EXCEPTION("timeout",exc);
        }
        
        CI_LOG_D(buffer);
        mReadBuffer = false;
        mSerial->flush();
    }
    
}

void cinderSandboxApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
}

CINDER_APP( cinderSandboxApp, RendererGl(RendererGl::Options().msaa(16)), [] (App::Settings * settings){
    settings->setHighDensityDisplayEnabled(false);

})
