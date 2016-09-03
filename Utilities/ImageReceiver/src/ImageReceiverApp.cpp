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
// the original image is pixelated by 20;

#define BUF_SIZE 80
#define READ_INTERVAL 0.15
#define BAUD_RATE 192000
#define NUM_BYTES 1

const ci::ivec2 margin(5,5);

class ImageReceiverApp : public App {
public:
    void setup() override;
    void mouseDown( MouseEvent event ) override;
    void keyDown(KeyEvent event) override;
    void update() override;
    void draw() override;
    
    
private:
    
    void initPort();
    int  coordToIndex(const int x, const int y);
    ivec2  indexToCoord(const int id);
    
    Surface             mSentImage;
    Surface8u           mReceivedImage;
    gl::Texture2dRef    mTexture;
    gl::Texture::Format mFormat;
    
    bool                mIsRecord;
    
    // Serial
    SerialRef           mSerial;
    bool                mIsReceiveMessage;
    
    // FBO
    // gl::FboRef          mFbo; // we don't need this!
    
    Font                mFont;
    gl::Texture2dRef    mTextTexture;
    
    // intervals
    double              mLastUpdate;
    double              mLastRead;
    string              mLastString;
    
    // pixel_values
    int                 mPixelCount;
    ivec2               mPixelCursor;
};

void ImageReceiverApp::setup()
{
    mSentImage = loadImage(loadAsset("wave.png")); // kinda cheating but none the less...
    ivec2 receivedImageSize = mSentImage.getSize() / stepDiv;
    
    // This holds what got from them...
    mReceivedImage = Surface(receivedImageSize.x,receivedImageSize.y,false);
    
    Surface8u::Iter iter(mReceivedImage.getIter());
    while(iter.line()){
        while(iter.pixel()){
            iter.r() = 0;
            iter.g() = 0;
            iter.b() = 0;
        }
    }
    
    
    mFormat.setMagFilter(GL_NEAREST); // we want crispy pixels
    
    setWindowSize(mSentImage.getSize()+margin*2);
    
    mIsRecord = false;
    
    mFont = Font("Arial",10);
    
    initPort();
    
    // timer stuff
    mLastUpdate = 0;
    mLastRead = 0;
    
    mPixelCount = 0;
    mPixelCursor = ivec2(indexToCoord(mPixelCount)*stepDiv+margin);
    
}

void ImageReceiverApp::mouseDown( MouseEvent event )
{
}

void ImageReceiverApp::keyDown(KeyEvent event)
{
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
        
//        try{
//            // read until newline, to a maximum of BUFSIZE bytes
//            mLastString = mSerial->readStringUntil( '\n', BUF_SIZE );
//            
//        }
//        catch( SerialTimeoutExc &exc ) {
//            CI_LOG_EXCEPTION( "timeout", exc );
//        }
        mLastString = mSerial->readStringUntil('\n',BUF_SIZE);
        
        mIsReceiveMessage = false;
        
        TextLayout simple;
        simple.setFont( mFont );
        if(mLastString.find("L: r=")!= string::npos ){
            
            string string_bits = mLastString.substr(5);
            uint8_t value = 0;
            for (int i =0;i<8;i++){
                value |= (string_bits[i]=='1') << (7-i);
            }
            
            CI_LOG_D("got value: "<< std::to_string(value));
            
            simple.setColor(Color::white());
            simple.addLine(std::to_string(value));
            
            mReceivedImage.setPixel(indexToCoord(mPixelCount),Color8u::gray(value));

            
            mPixelCount++;
            mPixelCursor = ivec2(indexToCoord(mPixelCount)*stepDiv+margin);
            
            mIsRecord = false;
            
            // update mTexture
            mTexture = gl::Texture::create(mReceivedImage,mFormat);
            
        }else if(mLastString.find("start")!=string::npos){
            CI_LOG_D("record start");
            
            mIsRecord = true;
            simple.setColor(Color(1.0,0.0,0.0)); // red
            simple.addLine("rec");
        }
        
        simple.setLeadingOffset( 0 );
        mTextTexture = gl::Texture::create( simple.render( true, false ) );
        
        mSerial->flush();
    }
    
    gl::clear(Color::black());
    
}

void ImageReceiverApp::draw()
{
    gl::clear(Color::black());
    
    gl::color(Color::white());
    
    //stuff
    if(mTexture){
        gl::draw(mTexture,Area(margin,margin+mSentImage.getSize()));
    }
    
    if(mTextTexture){
        gl::draw(mTextTexture,ivec2(margin.x,getWindowHeight()-margin.y));
    }
    
    // border
    gl::drawStrokedRect(Rectf(margin,margin+mSentImage.getSize()));
    
    // cursor
    if(mIsRecord) gl::color(Color(1,0,0));
    gl::drawStrokedRect(Rectf(mPixelCursor,mPixelCursor+stepDiv));

    
}


void ImageReceiverApp::initPort(){
    
    try {
        Serial::Device dev = Serial::findDeviceByNameContains( "tty.usbserial" );
        mSerial = Serial::create( dev, BAUD_RATE );
        CI_LOG_D("connected to " << dev.getName());
    }
    catch( SerialExc &exc ) {
        CI_LOG_EXCEPTION( "could not initialize the serial device", exc );
        exit( -1 );
    }
    
}

int ImageReceiverApp::coordToIndex(int x, int y){
    return y*mReceivedImage.getSize().x + x;
}

ivec2 ImageReceiverApp::indexToCoord(int id){
    int imageWidth = mReceivedImage.getSize().x;
    return ivec2(id%imageWidth,id/imageWidth);
}


CINDER_APP( ImageReceiverApp, RendererGl )
