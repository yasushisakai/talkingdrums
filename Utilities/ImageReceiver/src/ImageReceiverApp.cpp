#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Serial.h"
#include "cinder/Log.h"


#include <vector>

using namespace ci;
using namespace ci::app;
using namespace std;

const ci::ivec2 stepDiv(20,20);

#define BUF_SIZE 80
#define READ_INTERVAL 0.25
#define BAUD_RATE 9600
#define NUM_BYTES 1

//const ivec2 windowSize(1280+640,720);

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
    
    int                 mPixelWidth;
    bool                mIsRecord;
    
    // Serial
    SerialRef           mSerial;
    bool                mIsReceiveMessage;
    
    // FBO
    gl::FboRef          mFbo;
    
    Font                mFont;
    gl::Texture2dRef    mTextTexture;
    
    // intervals
    double              mLastUpdate;
    double              mLastRead;
    string              mLastString;
    
    // pixel_values
    int                 mPixelCount;
    vector<uint8_t>     mPixelValues; // this can be a fixed number
    
};

void ImageReceiverApp::setup()
{
    mSendImage = loadImage(loadAsset("wave.png"));
    ivec2 windowSize = mSendImage.getSize();
    windowSize.x += stepDiv.x*2;
    windowSize.y += stepDiv.y*2;
    
    setWindowSize(windowSize);
    
    mPixelWidth = mSendImage.getSize().x/stepDiv.x;
    
    mIsRecord = false;
    
    
    initFBO();
    initPort();
    
    
    mFont = Font("Arial",14);
    
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
    
    double now = getElapsedSeconds();
    double deltaTime = now - mLastUpdate;
    mLastUpdate = now;
    mLastRead += deltaTime;
    
    if( mLastRead > READ_INTERVAL )	{
        mIsReceiveMessage = true;
        mLastRead = 0.0;
    }
    
    if( mIsReceiveMessage ) {
        
        try{
            // read until newline, to a maximum of BUFSIZE bytes
            mLastString = mSerial->readStringUntil( '\n', BUF_SIZE );
            
        }
        catch( SerialTimeoutExc &exc ) {
            CI_LOG_EXCEPTION( "timeout", exc );
        }
        
        mIsReceiveMessage = false;
        
        //console() << mLastString << endl;
        
        
        TextLayout simple;
        simple.setFont( Font( "Arial Black", 10 ) );
        if(mLastString.find("L: r=")!= string::npos ){
            simple.setColor(Color(1.0,0,0));
            
            string string_bits = mLastString.substr(5);
            uint8_t value = 0;
            for (int i =0;i<8;i++){
                value += (string_bits[i]=='1') << (7-i);
            }
            
            mPixelValues.push_back(value);
            
            console()<< "got value: "<< std::to_string(value) << endl;
            simple.addLine(std::to_string(value));
            mIsRecord = false;
            mPixelCount++;
            
        }else if(mLastString.find("start")!=string::npos){
            console()<< "rec started " << endl;
            mIsRecord = true;
        }
        
        //simple.setColor( Color( .7, .7, .2 ) );
        simple.setLeadingOffset( 0 );
        mTextTexture = gl::Texture::create( simple.render( true, false ) );
        
        mSerial->flush();
    }
    
}

void ImageReceiverApp::draw()
{
    gl::clear( Color( 0, 0, 0 ) );
    
    if(mTextTexture){
        gl::draw(mTextTexture,vec2(stepDiv.x*2,stepDiv.y/2));
    }
    
    for(int i =0;i<mPixelValues.size();++i){
        gl::color(Color8u(mPixelValues[i],mPixelValues[i],mPixelValues[i]));
        int x = i%mPixelWidth;
        int y = i/mPixelWidth;
        Rectf rect = Rectf((x+2)*stepDiv.x,(y+2)*stepDiv.y,(x+3)*stepDiv.x,(y+3)*stepDiv.y);
        gl::drawSolidRect(rect);
    }
    
    int x= mPixelCount%mPixelWidth;
    int y= mPixelCount/mPixelWidth;
    
    if(!mIsRecord)
        gl::color(1, 1, 1);
    else
        gl::color(1.0,0.0,0.0);
    Rectf rect = Rectf((x+2)*stepDiv.x,(y+2)*stepDiv.y,(x+3)*stepDiv.x,(y+3)*stepDiv.y);
    gl::drawStrokedRect(rect);
    
}


void ImageReceiverApp::initPort(){
    
    try {
        Serial::Device dev = Serial::findDeviceByNameContains( "tty.usbserial" );
        mSerial = Serial::create( dev, BAUD_RATE );
        console() << "connected to " << dev.getName() << endl;
    }
    catch( SerialExc &exc ) {
        CI_LOG_EXCEPTION( "could not initialize the serial device", exc );
        exit( -1 );
    }
    
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
