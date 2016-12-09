#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Serial.h"
#include "cinder/Log.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/params/Params.h"
#include "cinder/Rand.h"

#include <bitset>
#include <vector>
#include <iostream>

//Defines
#define BUF_SIZE 80
#define BAUD_RATE 115200
#define NUM_BYTES 1
#define READ_INTERVAL 0.25

#define NAME_PORT "cu.usbserial-A700fbuz"

//name spaces
using namespace ci;
using namespace ci::app;
using namespace std;

//const values
const ci::ivec2 windowSize(1280 + 640, 720);

//const ci::ivec2 stepDiv(80, 80); //wave 80
//const string IMAGE_NAME = "wave.png";

const ci::ivec2 stepDiv(160, 160); //wave 80
const string IMAGE_NAME = "grid.png"; //wave.png

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
    
    //helpers
    int                 coordToIndex(const int x, const int y);
    ivec2               indexToCoord(const int id);
    void                cleanReceivedImage();
    Surface             processPixeletedImage(const Surface input, ci::ivec2 stepAmount, ci::ivec2 & numPixels);
    
    uint8_t             floatColorToInt(float inVal);
    int                 stringToInt(const std::string &s);
    
    
    void                renderOutputImage();
    
    Surface8u           mSentImage;
    Surface8u           mReceivedImage;
    
    ci::ColorA8u        mReceiveColor;
    
    Surface             mDebugImage;
    gl::Texture2dRef    mReceiveTex;
    Area                mTexBounds;
    gl::Texture::Format mFormatTex;
    
    string              mStrReceived;
    

    // Serial
    SerialRef           mSerial;
    bool                mIsReceiveMessage;
    bool                mInitPort;
    void                initPort();
    void                processInValue(double now);
    int                 mGrayReceived;
    bool                mGetPixels;
    bool                mSendSerialMessage;
    
    void                renderInputImage();
    
    // FBO
    gl::FboRef          mFbo; // we don't need this!
    Font                mFont;
    gl::Texture2dRef    mTextTexture;
    void                initFBO();
    
    // intervals
    double              mLastUpdate;
    double              mLastRead;
    string              mLastString;
    
    // iterate though the image and send it via serial port
    ci::ivec2           mIteraPixel;
    void                iterateBox();
    
    //debug
    bool                mDebug;
    gl::Texture2dRef    mDebugTex;
    ci::ivec2           mNumPixels;


    bool                mIsRecord;
    bool                mIsPlay;
    bool                mDone;
    
    //params gui
    params::InterfaceGlRef	mParams;
};

void ImageReceiverApp::initFBO()
{
    //FBO
    mFbo = gl::Fbo::create(mDebugImage.getWidth(), mDebugImage.getHeight());
    {
        gl::ScopedFramebuffer fbScp( mFbo );
        
        gl::clear( Color( 0.0, 0.0, 0.0 ) );
        
        gl::ScopedViewport scpVp( ivec2( 0 ), mFbo->getSize() );
        
    }
}


//start port
void ImageReceiverApp::initPort(){
    
    auto devices = Serial::getDevices();
    for(auto & ports : devices){
        console()<< ports.getName() <<std::endl;
    }
    
    try {
        if(devices.size() >= 2){
            auto dev = Serial::findDeviceByName(NAME_PORT);
            //  auto dev = devices[ devices.size() - 2 ];
            
            mSerial = Serial::create(dev, BAUD_RATE );
            CI_LOG_D("connected to " << dev.getName());
            mInitPort = true;
        }
    }
    catch( SerialExc &exc ) {
        CI_LOG_EXCEPTION( "could not initialize the serial device", exc );
        mInitPort = false;
        //exit( -1 );
    }
    
}
void ImageReceiverApp::setup()
{
    
    setWindowSize(windowSize);
    
    //debug
    mDebugImage = loadImage(loadAsset(IMAGE_NAME));
    //mDebugSurface = processPixeletedImage(mDebugImage, stepDiv, mNumPixels);
    
    ci::ivec2 centerDiv( stepDiv.x / 2.0, stepDiv.y /2.0);
    ci::ivec2 imageSize(mDebugImage.getWidth(), mDebugImage.getHeight());
    mNumPixels = imageSize/stepDiv;
    
    mDebugTex = gl::Texture2d::create(mDebugImage);
    
    //initialize port
    initPort();
    
    // we want crispy pixels
    mFormatTex.setMagFilter(GL_NEAREST);
    
    //FONT
    mFont = Font("Arial", 20);
    
    //FBO
    initFBO();
    
    // timer stuff
    mLastUpdate = 0;
    mLastRead = 0;
    
    //start  - 1
    mIteraPixel   = ci::ivec2(-1, 0);
    mReceiveColor = ci::ColorA8u(0, 0, 0);
    
    mIsRecord  = false;
    mIsPlay    = true;
    mDone      = false;
    mDebug     = true;
    mGetPixels = false;
    
    mLastString = "";
    
    mStrReceived = "00000000";

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
        case 'x':
            mGetPixels = true;
            break;
        case '1':
            mGrayReceived = 100;
            mGetPixels = true;
            break;
            
        default:
            break;
    }
}

void ImageReceiverApp::renderOutputImage()
{
    if(mGetPixels){
        
              iterateBox();
        
        gl::ScopedFramebuffer fbScp( mFbo );
        
        gl::ScopedViewport scpVp( ivec2( 0 ), mTexBounds.getSize() );
        gl::ScopedMatrices matrices;
        gl::setMatricesWindow( mTexBounds.getSize(), true);
        gl::setModelMatrix(ci::mat4());
        
        gl::ScopedColor col;
        gl::ScopedMatrices mat;
        
        float xAspect =  ((stepDiv.x * mIteraPixel.x ) / (float) mDebugTex->getWidth() )* mTexBounds.getWidth();
        float yAspect =  ((stepDiv.y * mIteraPixel.y ) / (float) mDebugTex->getHeight() )* mTexBounds.getHeight();
        
        //inverted aspect ratio
        ci::vec2 aspectInv( (float)mTexBounds.getWidth()/ (float) mDebugTex->getWidth(), (float)mTexBounds.getHeight()/ (float) mDebugTex->getHeight() );
        
        gl::translate(0, 0);
        gl::translate(ci::vec2(xAspect, yAspect));
        gl::color(ci::ColorA8u(mGrayReceived, mGrayReceived, mGrayReceived));
        gl::drawSolidRect(Rectf(0, 0, stepDiv.x * aspectInv.x, stepDiv.y * aspectInv.y));
        //iterate
  
        mGetPixels = false;

    }
}

void ImageReceiverApp::update()
{
    
    double now = getElapsedSeconds();
    
    processInValue(now);
    
    renderOutputImage();
    
}

////----- DRAW
void ImageReceiverApp::draw()
{
    
    gl::clear(Color::black());
    
    
    if(mDebug){
        
        if(mDebugTex){
            {
                gl::ScopedColor col;
                gl::ScopedMatrices  mat;
                gl::color(1, 1, 1);
                gl::translate(ci::ivec2( 0.0*(getWindowWidth()/3.0), getWindowHeight()/5.0));
                gl::draw(mDebugTex, mTexBounds);
            }
            
    
            
            //draw text
            TextLayout simple;
            simple.setFont( mFont );
            simple.setColor( Color( 0.8, 0.8, 0.8f ) );
            
            //draw the pixels
            if(mIteraPixel.x >= 0 && mIteraPixel.y >= 0){
                ci::ivec2 mCentralPixel = ci::vec2(mIteraPixel) * ci::vec2(stepDiv.x, stepDiv.y) + ci::vec2(stepDiv.x/2.0, stepDiv.y/2.0);
                //CI_LOG_V(mCentralPixel);
                ci::ColorA realCol =  mDebugImage.getPixel(mCentralPixel);
                
                std::string  realColorStr = "r: ("+ to_string(floatColorToInt(realCol.r))+", "+to_string(floatColorToInt(realCol.g))+", "+ to_string(floatColorToInt(realCol.b))+")";
                
                std::string indexStr = "current ["+to_string(mIteraPixel.x)+", "+to_string(mIteraPixel.y)+"]";
            
            
                //float lumaReceive = 0.2126 * mReceiveColor.r + 0.7152 * mReceiveColor.g + 0.0722 * mReceiveColor.b;
                //float lumaReal = 0.2126 * realCol.r + 0.7152 * realCol.g + 0.0722 * realCol.b;
                //uint8_t grayReceiver = floatColorToInt(lumaReceive);
                //uint8_t grayReal     = floatColorToInt(lumaReal);
                
                
                uint8_t resultReceived[8];
                uint8_t resultReal[8];
                
                std::string byteRec;
                std::string byteReal;
                
                uint8_t realGray     = floatColorToInt(realCol.r);
                
                for(int i = 0; i < 8; ++i) {
                    resultReceived[i] = 0 != (mGrayReceived & (1 << i));
                    byteRec += to_string(resultReceived[i]);
                }
                
                for(int i = 0; i < 8; ++i) {
                    resultReal[i] = 0 != (realGray & (1 << i));
                    byteReal += to_string(resultReal[i]);
                }
                
                string bytesComp = "in "+mStrReceived +" - r: "+ byteReal;
            
            
                simple.addCenteredLine(realColorStr) ;
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
    }
    
    //middle moving block
    //moving block
    
    if(mFbo->getColorTexture()){
        gl::ScopedMatrices  mat;
        gl::translate(ci::ivec2( 1.0*(getWindowWidth()/3.0), -getWindowHeight()/3.333));
        gl::draw(mFbo->getColorTexture());
    }
    
    

    mParams->draw();
    
}

void ImageReceiverApp::processInValue(double now)
{
    
    double deltaTime = now - mLastUpdate;
    mLastUpdate = now;
    mLastRead += deltaTime;
    
    if( mLastRead > READ_INTERVAL )	{
        mSendSerialMessage = true;
        mLastRead = 0.0;
    }

    if( mSendSerialMessage ) {
        if(mInitPort){
            
            try{
                // read until newline, to a maximum of BUFSIZE bytes
                mLastString = mSerial->readStringUntil( '\n', BUF_SIZE );
            }
            catch( SerialTimeoutExc &exc ) {
               CI_LOG_EXCEPTION( "timeout", exc );
            }
            
            //string
            CI_LOG_D(mLastString);
            
            if(!mLastString.empty()){
                if(mLastString.find("L: r=")!= string::npos && mLastString.size() >= 5){
                    
                    //get "L: r="
                    string string_bits = mLastString.substr(5);
                    int value = stringToInt(string_bits);
                    mGrayReceived = value;
                    mStrReceived = string_bits;
                    
                    CI_LOG_D("got value: "<<" "<<string_bits<<" "<<mStrReceived<<" "<<mGrayReceived);
                    
                    //mGrayReceived = value;
                    
                    
                    mGetPixels = true;
                    
                }else if(mLastString.find("L: found head")!=string::npos){
                    CI_LOG_D("found head");
                    mIsRecord = true;
                }else if(mLastString.find("end")!= string::npos){
                    CI_LOG_D("got pixel");
                    mIsPlay = false;
                }
            }
        }
        mSendSerialMessage = false;
    }
}

void ImageReceiverApp::iterateBox(){
    mIteraPixel.x++;
    if(mIteraPixel.x >= mNumPixels.x){
        mIteraPixel.y++;
        mIteraPixel.x = 0;
    }
    
    if(mIteraPixel.y >= mNumPixels.y){
        mIteraPixel = ci::vec2(0, 0);
        CI_LOG_D("DONE RECEIVING PIXEL");
    }
    

    console()<<mIteraPixel<<std::endl;

}




void ImageReceiverApp::cleanReceivedImage()
{
    
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

//process image
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
                
                CI_LOG_V(i<<" "<< j <<" "<<counterPix);
                CI_LOG_V("center "<<xyIter);
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

void ImageReceiverApp::saveImage(){
    writeImage(getHomeDirectory() / "Desktop" / "out.png", copyWindowSurface());
    CI_LOG_D("image saved");
}

uint8_t ImageReceiverApp::floatColorToInt(float inVal)
{
    union { float f; uint32_t i; } u;
    u.f = 32768.0f + inVal * (255.0f / 256.0f);
    return (uint8_t)u.i;
}

int ImageReceiverApp::stringToInt(const std::string &s)
{
    uint b = 0;
    for (int i = 7; i >=0; --i)
    //for (int i = 0; i < 8; i++)
    {
        b <<= 1;
        if (s.at(i) == '1')
            b |= 1;
    }
    console()<<b<<std::endl;
    int num;
    return (num = (int)b);
    
    
}

CINDER_APP( ImageReceiverApp, RendererGl( RendererGl::Options().msaa( 16 ) ), [] (App::Settings * settings){
    
    //settings->setHighDensityDisplayEnabled();
})
