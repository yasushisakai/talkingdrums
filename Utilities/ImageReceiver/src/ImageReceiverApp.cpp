#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Serial.h"
#include "cinder/Log.h"
//#include "cinder/params/Params.h"

using namespace ci;
using namespace ci::app;
using namespace std;


#define BUF_SIZE 80;
#define READ_INTERVAL 0.25;
#define BAUD_RATE 9600;
#define NUM_BYTES 1;

const ivec2 windowSize(1280+640,720);

class ImageReceiverApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
    void keyDown(KeyEvent event) override;
	void update() override;
	void draw() override;
    
    
  private:
    void initPort();
    void initFBO();
    
    Surface             mSendImage;
    Surface             mPixelImage;
    
    // Serial
    SerialRef           mSerial;
    string              mSerialName;
    string              mSerialStr;
    uint8_t*            mSendData;
    
    // FBO
    gl::FboRef          mFbo;
    
    
    ivec2               mIteraPixel;
    ColorA              mCurrentColor; // can this be char??
    bool                mFinishSending;
    bool                mPixelReady;
    bool                mSendPixels;
    bool                mDrawOriginal;
    
    Font                mFont;
    gl::Texture2dRef    mTextTexture;
    
//    params::InterfaceGlRef mParams;
//    float               mAvgFps;
    
    // intervals
    double              mLastUpdate;
    double              mLastRead;
    

};

void ImageReceiverApp::setup()
{
    setWindowSize(windowSize);
    mSendImage = loadImage(loadAsset("wave.png"));
    
    initFBO();
    initPort();
    
    // we don't need to process the image.. just recieve it!
    
    mIteraPixel    = ivec2(0,0);
    mFinishSending = false;
    mPixelReady    = false;
    mSendPixels    = false;
    mDrawOriginal  = false;
    
    
    mFont = Font("Arial",20);
    mTextTexture = Area(0,0,width,height);
    
//    mParams = params::InterfaceGlRef::create(getWindow(),"App Parameters", toPixels(ivec(200,200)));
//    mParams->addParam("FPS",&mAvgFps,false);
//    mParams->addParam("Draw",&mDrawOriginal);
    
    mLastUpdate = 0;
    mLastRead = 0;
    
}

void ImageReceiverApp::mouseDown( MouseEvent event )
{
}

void ImageReceiverApp::keyDown(KeyEvent event){
    
}

void ImageReceiverApp::update()
{
    mAvgFps = getAverageFps();
    renderOutputImage();
    
    double now = getElapsedSeconds();
    double delta_time = now - mLastUpdate;
    mLastUpdate = now;
    mLastRead += delta_time;
    
    if(mLastRead > READ_INTERVAL){
        mLastString = mSerial->readStringUntill('\n',BUFSIZE);
        
        mLastRead = 0;
    }
    
    
    
    
}

void ImageReceiverApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    
}


void ImageReceiverApp::initPort(){
    // print the devices
    string findUsb = "cu.usbserial";
    for( const auto &dev : Serial::getDevices() ){
        CI_LOG_V("Device: " << dev.getName() );
    }try {
        Serial::Device dev = Serial::findDeviceByNameContains( "cu.usbserial" );
        mSerialName = dev.getName();
        mSerial = Serial::create( dev, BAUD_RATE );
        CI_LOG_I( "Connected to: "<< mSerialName);
    }
    catch( SerialExc &exc ) {
        CI_LOG_EXCEPTION( "coult not initialize the serial device", exc );
        exit( -1 );
    }
    mSendData = new uint8_t[NUM_BYTES];
}

void ImageReceiverApp::initFBO(){
    //FBO
    mFbo = gl::Fbo::create(mSendImage.getWidth(), mSendImage.getHeight());
    {
        gl::ScopedFramebuffer fbScp( mFbo );
        
        gl::clear( Color( 0.0, 0.0, 0.0 ) );
        
        gl::ScopedViewport scpVp( ivec2( 0 ), mFbo->getSize() );
        
    }
}


CINDER_APP( ImageReceiverApp, RendererGl )
