#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Serial.h"
#include "cinder/Log.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/params/Params.h"
#include "cinder/Rand.h"


#include <vector>

//Defines
#define BUF_SIZE 80
#define READ_INTERVAL 0.25
#define BAUD_RATE 19200
#define NUM_BYTES 1

//name spaces
using namespace ci;
using namespace ci::app;
using namespace std;

//const values
const ci::ivec2 windowSize(1280 + 640, 720);

const ci::ivec2 stepDiv(100, 100);
// the original image is pixelated by 20;

const ci::ivec2 margin(5,5);

class ImageReceiverApp : public App {
public:
    void setup() override;
    void mouseDown( MouseEvent event ) override;
    void keyDown(KeyEvent event) override;
    void update() override;
    void draw() override;
    void saveImage();
    
private:
    
    int  coordToIndex(const int x, const int y);
    ivec2  indexToCoord(const int id);
    
    Surface8u           mSentImage;
    Surface8u           mReceivedImage;
    
    void                cleanReceivedImage();
    
    ci::ColorA8u        mReceiveColor;
    ci::ivec2           mPixelCount;
    
    
    gl::Texture2dRef    mReceiveTex;
    Area                mTexBounds;
    gl::Texture::Format mFormatTex;
    

    // Serial
    SerialRef           mSerial;
    bool                mIsReceiveMessage;
    bool                mInitPort;
    void                initPort();
    void                processPort(double now);
    
    // FBO
    // gl::FboRef          mFbo; // we don't need this!
    
    Font                mFont;
    gl::Texture2dRef    mTextTexture;
    
    // intervals
    double              mLastUpdate;
    double              mLastRead;
    string              mLastString;
    
    // pixel_values
    ivec2               mPixelCursor;
    
    //debug
    bool                mDebug;
    Surface             mDebugSurface;
    gl::Texture2dRef    mDebugTex;
    ci::ivec2           mNumPixels;
    
    
    Surface processPixeletedImage(const Surface input, ci::ivec2 stepAmount, ci::ivec2 & numPixels);
    
    enum                State {
        WAIT_START,
        LISTEN,
        ANALYZE,
        HEADER_PLAY,
        PULSE_PLAY,
        WAIT_PLAY,
        RESET
    };
    State               sequenceState;
    
    bool                mIsRecord;
    bool                mIsPlay;
    bool                mDone;
    
    uint8_t             floatColorToInt(float inVal);
    
    //params gui
    params::InterfaceGlRef	mParams;
};

void ImageReceiverApp::setup()
{
    
    setWindowSize(windowSize);
    
    //initialize port
    initPort();
    
    // we want crispy pixels
    mFormatTex.setMagFilter(GL_NEAREST);
    
    //FONT
    mFont = Font("Arial", 20);
    
    // timer stuff
    mLastUpdate = 0;
    mLastRead = 0;
    
    mPixelCount   = ci::ivec2(0, 0);
    mPixelCursor  = ci::ivec2(0, 0);
    mReceiveColor = ci::ColorA8u(0, 0, 0);
    
    //Sequence
    sequenceState = WAIT_START;
    
    mIsRecord = false;
    mIsPlay   = true;
    mDone     = false;
    mDebug    = true;
    
    //debug
    Surface mDebugImage = loadImage(loadAsset("wave.png"));
    mDebugSurface = processPixeletedImage(mDebugImage, stepDiv, mNumPixels);
    mDebugTex = gl::Texture2d::create(mDebugSurface);
    
    // This holds what got from them...
    mReceivedImage = Surface8u(mNumPixels.x, mNumPixels.y, false);
    mReceiveTex  =  gl::Texture2d::create(mReceivedImage, mFormatTex);
    
    //file image with black
    cleanReceivedImage();

    //calculate height depending on the aspect ratio of the image
    //scaling ??
    float width  = getWindowWidth()/3.0;
    float height =  (width * mDebugTex->getHeight() ) /(mDebugTex->getWidth());
    mTexBounds   = ci::Area(0, 0, width, height);
    
    CI_LOG_I(width<<" "<<height);

    
    //create params
    mParams = params::InterfaceGl::create( getWindow(), "App parameters", toPixels( ivec2( 200, 200 ) ) );
    mParams->addParam("Debug",  &mDebug);
}

void ImageReceiverApp::mouseDown( MouseEvent event )
{
    
}

void ImageReceiverApp::keyDown(KeyEvent event)
{
    switch (event.getChar()) {
        case 'a':
            cleanReceivedImage();
            break;
        case 's':
            saveImage();
            break;
            
            
        default:
            break;
    }
}

void ImageReceiverApp::update()
{
    
    double now = getElapsedSeconds();
    
    processPort(now);
    
    gl::clear(Color::black());
    
}

void ImageReceiverApp::draw()
{
    
    gl::clear(Color::black());
    
    if(mReceiveTex){
        gl::ScopedColor col;
        gl::ScopedMatrices  mat;
        gl::color(1, 1, 1);
        gl::translate(ci::ivec2( 0.0*(getWindowWidth()/3.0), getWindowHeight()/5.0));
        gl::draw(mReceiveTex, mTexBounds);
    }
    
    
    if(mDebug){
        if(mDebugTex){
            {
                gl::ScopedColor col;
                gl::ScopedMatrices  mat;
                gl::color(1, 1, 1);
                gl::translate(ci::ivec2( 1.0*(getWindowWidth()/3.0), getWindowHeight()/5.0));
                gl::draw(mDebugTex, mTexBounds);
            }
            
            
            //draw text
            TextLayout simple;
            simple.setFont( mFont );
            simple.setColor( Color( 0.8, 0.8, 0.8f ) );
            
            ci::ColorA realCol = mDebugSurface.getPixel(ci::ivec2(mPixelCount));
            
            std::string  rgbColorStr = "("+ to_string(floatColorToInt(mReceiveColor.r))+", "+to_string(floatColorToInt(mReceiveColor.g))+", "+ to_string(floatColorToInt(mReceiveColor.b))+")";
            std::string  realColorStr = "("+ to_string(floatColorToInt(realCol.r))+", "+to_string(floatColorToInt(realCol.g))+", "+ to_string(floatColorToInt(realCol.b))+")";
            
            std::string indexStr = "["+to_string(mPixelCount.x)+", "+to_string(mPixelCount.y)+"]";
            
            //
            float lumaReceive = 0.2126 * mReceiveColor.r + 0.7152 * mReceiveColor.g + 0.0722 * mReceiveColor.b;
            float lumaReal = 0.2126 * realCol.r + 0.7152 * realCol.g + 0.0722 * realCol.b;
            
            uint8_t grayReceiver = floatColorToInt(lumaReceive);
            uint8_t grayReal     = floatColorToInt(lumaReal);
            
            
            uint8_t resultReceive[8];
            uint8_t resultReal[8];
            
            std::string byteRec;
            std::string byteReal;
            
            for(int i = 0; i < 8; ++i) {
                resultReceive[i] = 0 != (grayReceiver & (1 << i));
                byteRec += to_string(resultReceive[i]);
            }
            
            for(int i = 0; i < 8; ++i) {
                resultReal[i] = 0 != (grayReal & (1 << i));
                byteReal += to_string(resultReal[i]);
            }
            
            string bytesComp = byteRec +" - "+ byteReal;
        
        
        
            simple.addLine(rgbColorStr +" - "+ realColorStr) ;
            simple.addCenteredLine(indexStr);
            
            simple.addCenteredLine(bytesComp);
            mTextTexture = gl::Texture2d::create( simple.render( true, false ) );
            
            if(mTextTexture){
                gl::ScopedMatrices  mat;
                gl::ScopedColor col;
                gl::translate(ci::vec2(mTexBounds.getWidth()/2.5 + 1.0*(getWindowWidth()/3.0), mTexBounds.getHeight() +  getWindowHeight()/5.0 ));
                gl::color(0.6, 0.6, 0.6);
                gl::draw(mTextTexture, vec2(0, 0));
            }

        }
    }
    
    // border
   // gl::drawStrokedRect(Rectf(margin,margin+mSentImage.getSize()));
    
    // cursor
    if(mIsRecord){
        gl::color(Color(1, 0, 0));
    }else if(mIsPlay){
        gl::color(Color(0, 1, 0));
    }
    
    gl::drawStrokedRect(Rectf(mPixelCursor, mPixelCursor + stepDiv));
    
    mParams->draw();
    
}

void ImageReceiverApp::processPort(double now)
{
    if(mInitPort){
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
            //mLastString = mSerial->readStringUntil('\n',BUF_SIZE);
            
            
            CI_LOG_D(mLastString);
            
            mIsReceiveMessage = false;
            
            TextLayout simple;
            simple.setFont( mFont );
            if(mLastString.find("L: r=")!= string::npos ){
                
                string string_bits = mLastString.substr(5);
                uint8_t value = 0;
                for (int i = 0;i < 8; i++){
                    value |= (string_bits[i]=='1') << (7-i);
                }
                
                CI_LOG_D("got value: "<< std::to_string(value));
                
                simple.setColor(Color::white());
                simple.addLine(std::to_string(value));
                
                mReceiveColor = Color8u::gray(value);
                
                mReceivedImage.setPixel(mPixelCount, mReceiveColor);
                
                mPixelCount.x++;
                if(mPixelCount.x > mNumPixels.x){
                    mPixelCount.y++;
                    mPixelCount.x = 0;
                }
                
                //finish getting the image
                if(mPixelCount.y > mNumPixels.y){
                    mDone = true;
                    
                }
                
                mPixelCursor = ivec2(mPixelCount.x * stepDiv.x, mPixelCount.y * stepDiv.y);
                
                mIsRecord = false;
                mIsPlay = true;
                
                // update mTexture
                mReceiveTex = gl::Texture::create(mReceivedImage, mFormatTex);
                
            }else if(mLastString.find("found head")!=string::npos){
                CI_LOG_D("found head");
                
                mIsRecord = true;
                simple.setColor(Color(1.0,0.0,0.0)); // red
                simple.addLine("rec");
            }else if(mLastString.find("end")!= string::npos){
                CI_LOG_D("got pixel");
                
                mIsPlay = false;
            }
            
            simple.setLeadingOffset( 0 );
            mTextTexture = gl::Texture::create( simple.render( true, false ) );
            
            //mSerial->flush();
        }
    }
    
}

void ImageReceiverApp::saveImage(){
    writeImage(getHomeDirectory() / "Desktop" / "out.png", copyWindowSurface());
    CI_LOG_D("image saved");
}

//start port
void ImageReceiverApp::initPort(){
    
    try {
        Serial::Device dev = Serial::findDeviceByNameContains( "cu.usbserial" );
        mSerial = Serial::create( dev, BAUD_RATE );
        CI_LOG_D("connected to " << dev.getName());
        mInitPort = true;
    }
    catch( SerialExc &exc ) {
        CI_LOG_EXCEPTION( "could not initialize the serial device", exc );
        mInitPort = false;
        //exit( -1 );
    }
    
}

void ImageReceiverApp::cleanReceivedImage()
{
    
    mPixelCount = ci::ivec2(0, 0);
    
    for(int i = 0; i < mReceivedImage.getSize().x; i++){
        for(int j = 0; j < mReceivedImage.getSize().y; j++){
            mReceivedImage.setPixel(ci::ivec2(i, j), ci::ColorA8u(0, 0, 0));
        }
    }
    
    console()<<"clean images"<<std::endl;

}

int ImageReceiverApp::coordToIndex(int x, int y){
    return y*mReceivedImage.getSize().x + x;
}

ivec2 ImageReceiverApp::indexToCoord(int id){
    int imageWidth = mReceivedImage.getSize().x;
    return ivec2(id%imageWidth,id/imageWidth);
}

Surface ImageReceiverApp::processPixeletedImage(const Surface input, ci::ivec2 stepAmount, ci::ivec2 & numPixels)
{
    
    ci::Area areaProcess = input.getBounds();
    Surface pixelImage =  input.clone( areaProcess );
    
    //initial values
    int counterPix = 0;
    ci::ivec2 centerDiv( stepAmount.x / 2.0, stepAmount.y /2.0);
    ci::ivec2 xyIter(centerDiv.x, centerDiv.y);
    ci::ivec2 imageSize(areaProcess.x2, areaProcess.y2);
    numPixels = imageSize/stepAmount;
    
    
    CI_LOG_I("Center "<< centerDiv);
    CI_LOG_I(numPixels<<" "<<numPixels.x * numPixels.y);
    CI_LOG_I("Area "<<imageSize.x<<" "<<imageSize.y);
    
    //process the Image
    ci::ColorA col = input.getPixel(xyIter);
    
    for(int i = 0; i < imageSize.x; i++){
        for(int j = 0; j < imageSize.y; j++){
            
            //change color depending ifon the center pixel
            if(j % stepAmount.y == 0){
                counterPix++;
                
                xyIter.y += stepAmount.y;
                col = input.getPixel(xyIter);
                
                // CI_LOG_V(i<<" "<< j <<" "<<counterPix);
                // CI_LOG_V("center "<<xyIter);
            }
            
            
            pixelImage.setPixel(ci::ivec2(i, j), col);
            
        }
        
        xyIter.y = (int)centerDiv.y;
        
        if(i %  stepAmount.x == 0){
            xyIter.x += stepAmount.x;
        }
        
    }
    
    return pixelImage;
}

uint8_t ImageReceiverApp::floatColorToInt(float inVal)
{
    union { float f; uint32_t i; } u;
    u.f = 32768.0f + inVal * (255.0f / 256.0f);
    return (uint8_t)u.i;
}


CINDER_APP( ImageReceiverApp, RendererGl( RendererGl::Options().msaa( 16 ) ), [] (App::Settings * settings){
    
    //settings->setHighDensityDisplayEnabled();
})
