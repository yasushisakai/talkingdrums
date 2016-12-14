#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"
#include "cinder/CinderMath.h"
#include "cinder/Log.h"
#include "cinder/params/Params.h"
#include "cinder/Font.h"
#include "cinder/gl/Fbo.h"
#include "cinder/Serial.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define NAME_PORT "cu.usbserial-A700fbuz"

const ci::ivec2 windowSize(1280 + 640, 720);


//const ci::ivec2 stepDiv(80, 80); //wave 80
//const string IMAGE_NAME = "wave.png";

const ci::ivec2 stepDiv(40, 40); //wave 80
const string IMAGE_NAME = "music.png"; //wave.png


const int BAUD_RATE = 115200;


const int NUM_BYTES = 1;



class ImageSenderApp : public App {
public:
    void setup() override;
    void mouseDown( MouseEvent event ) override;
    void keyDown( KeyEvent event) override;
    void update() override;
    void draw() override;
    
    Surface processPixeletedImage(const Surface input, ci::ivec2 stepAmount, ci::ivec2 & numPixels);
    
    void processPixels(double currentTime);
    
    void initFBO();
    void renderOutputImage();
    
    //PORT
    void initPort();
    
    uint8_t floatColorToInt(float inVal);
    
private:
    
    void                updateIteration();
    void                renderTexFont();
    
    ci::Surface         mSendImage;
    
    gl::Texture2dRef    mSendTex;
    ci::Area            mTexBounds;
    ci::ivec2           mNumPixels;
    
    gl::Texture2dRef    mSendTexProces;
    
    
    //FBO for the output test image
    gl::FboRef			mFbo;
    
    
    // iterate though the image and send it via serial port
    ci::ivec2           mIteraPixel;
    ci::ColorA          mCurrentColor;
    ci::vec2            mCentralPixel;
    
    bool                mFinishSending;
    bool                mSendPixels;
    bool                mDrawOriginal;
    
    //fonts
    gl::Texture2dRef	mTextTexture;
    Font                mFont;
    
    //Serial
    SerialRef           mSerial;
    string              mSerialName;
    string              mSerialStr;
    double              mSerialPrevT;
    double              mSerialDuratinT;
    uint8_t *           mSendData;
    bool                mNexIteration;
    bool                mReadInMsg;
    bool                mSerialStarted;
    
    bool                mUpdateRender; //updating moving vox
    string              mCurrentByteStr;
    
    //Time Events
    double               mCurrentT;
    
    //params gui
    params::InterfaceGlRef	mParams;
    float                   mAvgFps;
    
    
    
};

void ImageSenderApp::initFBO()
{
    //FBO
    mFbo = gl::Fbo::create(mSendImage.getWidth(), mSendImage.getHeight());
    {
        gl::ScopedFramebuffer fbScp( mFbo );
        
        gl::clear( Color( 0.0, 0.0, 0.0 ) );
        
        gl::ScopedViewport scpVp( ivec2( 0 ), mFbo->getSize() );
        
    }
    
}

void ImageSenderApp::initPort()
{
    // print the devices
    
    auto devices = Serial::getDevices();
    for(auto & ports : devices){
        console()<< ports.getName() <<std::endl;
    }
    
    try {
        
         auto dev = Serial::findDeviceByName(NAME_PORT);
        mSerial = Serial::create(dev, BAUD_RATE );
        CI_LOG_D("connected to " << dev.getName());
        mSerialName =   dev.getName();
        mSerialStarted = true;
    }
    catch( SerialExc &exc ) {
        CI_LOG_EXCEPTION( "could not initialize the serial device", exc );
        mSerialStarted = false;
        //exit( -1 );
    }
    
    
    mSendData = new uint8_t[NUM_BYTES];
}

void ImageSenderApp::setup()
{
    setWindowSize(windowSize);
    
    mSendImage = loadImage(loadAsset(IMAGE_NAME));
    
    //FBO
    initFBO();
    
    //PORT
    initPort();
    
    //process the image
  //  mPixelImage = processPixeletedImage(mSendImage, stepDiv, mNumPixels);
    
    ci::ivec2 centerDiv( stepDiv.x / 2.0, stepDiv.y /2.0);
    ci::ivec2 imageSize(mSendImage.getWidth(), mSendImage.getHeight());
    mNumPixels = imageSize/stepDiv;
    
    
    //copy the surface to GPU Texture
    mSendTex = gl::Texture2d::create(mSendImage);
    mSendTexProces = gl::Texture2d::create(mSendImage);
    
    
    //calculate height depending on the aspect ratio of the image
    //scaling ??
    float width = getWindowWidth()/3.0;
    float height =  (width * mSendTex->getHeight() ) /(mSendTex->getWidth());
    
    CI_LOG_I(width<<" "<<height);
    
    //create font
    mFont =  Font( "Arial", 20 );
    
    //intial values
    mTexBounds = ci::Area(0, 0, width, height);
    
    mFinishSending = false;
    
    mDrawOriginal  = false;
    
    mNexIteration  = false;
    
    mReadInMsg     = true;
    
    mUpdateRender = false;
    
    mCurrentByteStr = "";
    
    mCurrentColor = ci::Color(1, 1, 1);
    
    //start in -1
    mIteraPixel    = ivec2(-1, 0);
    
    //create params
    mParams = params::InterfaceGl::create( getWindow(), "App parameters", toPixels( ivec2( 200, 200 ) ) );
    mParams->addParam("FPS", &mAvgFps, false);
    mParams->addParam("Draw", &mDrawOriginal);
    
}

Surface ImageSenderApp::processPixeletedImage(const Surface input, ci::ivec2 stepAmount, ci::ivec2 & numPixels)
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
                
                //CI_LOG_V(i<<" "<< j <<" "<<counterPix);
                //CI_LOG_V("center "<<xyIter);
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


void ImageSenderApp::mouseDown( MouseEvent event )
{
    
}

void ImageSenderApp:: keyDown( KeyEvent event)
{
    switch(event.getChar()){
        case 'a':
            mSendPixels = true;
            mUpdateRender = true;
            mCurrentColor =  mSendImage.getPixel(ci::ivec2(stepDiv.x/2, stepDiv.y/2));
            CI_LOG_I("START SENDING PIXELS");
            break;
        case 's':
            mNexIteration = true;
            break;
        case 'z':
            mNexIteration = false;
            mReadInMsg = true;
            break;
        case '1':
            mReadInMsg = true;
            break;
        case '2':
            //mCurrentColor = ci::Color(0, 1, 1);
            mUpdateRender = true;
            break;
    }
}


////----
void ImageSenderApp::update()
{
    mAvgFps = getAverageFps();
    
    mCurrentT = getElapsedSeconds();
    
    processPixels(mCurrentT);
    
    renderOutputImage();
    
}

void ImageSenderApp::renderOutputImage()
{
    if(mUpdateRender){
        
        {
            gl::ScopedFramebuffer fbScp( mFbo );
        
            
            gl::ScopedViewport scpVp( ivec2( 0 ), mTexBounds.getSize() );
            gl::ScopedMatrices matrices;
            gl::setMatricesWindow( mTexBounds.getSize(), true);
            gl::setModelMatrix(ci::mat4());
            
            gl::ScopedColor col;
            gl::ScopedMatrices mat;
            
            float xAspect =  ((stepDiv.x * mIteraPixel.x ) / (float) mSendTexProces->getWidth() )* mTexBounds.getWidth();
            float yAspect =  ((stepDiv.y * mIteraPixel.y ) / (float) mSendTexProces->getHeight() )* mTexBounds.getHeight();
            
            //inverted aspect ratio
            ci::vec2 aspectInv( (float)mTexBounds.getWidth()/ (float) mSendTexProces->getWidth(), (float)mTexBounds.getHeight()/ (float) mSendTexProces->getHeight() );
            
            gl::translate(0, 0);
            gl::translate(ci::vec2(xAspect, yAspect));
            gl::color(mCurrentColor);
            gl::drawSolidRect(Rectf(0, 0, stepDiv.x * aspectInv.x, stepDiv.y * aspectInv.y));
       
        }
        renderTexFont();
        mUpdateRender = false;
    }
}

////----
void ImageSenderApp::draw()
{
    gl::clear( Color( 0, 0, 0 ) );
    
    
    gl::ScopedMatrices matrices;
    gl::setMatricesWindow( getWindowSize(), true);
    
    //draw pixeleted image, the image that we are going to send
    if(!mDrawOriginal){
        if(mSendTexProces){
            gl::ScopedBlendAlpha alpha;
            
            gl::ScopedColor col;
            gl::ScopedMatrices  mat;
            gl::translate(ci::ivec2( 0.0*(getWindowWidth()/3.0), getWindowHeight()/5.0));
            
            //Alpha effect to texture
            if(mSendPixels){
                gl::enableAlphaBlending();
                gl::color(1.0, 1.0, 1.0, 0.5);
            }else{
                gl::color(1.0, 1.0, 1.0, 1.0);
            }
            gl::draw(mSendTexProces, mTexBounds);
            
        }
    }
    
    //draw original image
    if(mDrawOriginal){
        if(mSendTex){
            gl::ScopedMatrices  mat;
            gl::translate(ci::ivec2(0,  10));
            gl::draw(mSendTex, mTexBounds);
        }
        
        if(mSendTexProces){
            gl::ScopedBlendAlpha alpha;
            
            gl::ScopedColor col;
            gl::ScopedMatrices  mat;
            gl::translate(ci::ivec2( 0.0*(getWindowWidth()/3.0),  getWindowHeight()*0.52));
            
            //Alpha effect to texture
            if(mSendPixels){
                gl::enableAlphaBlending();
                gl::color(1.0, 1.0, 1.0, 1.0);
            }else{
                 gl::color(1.0, 1.0, 1.0, 1.0);
            }
            gl::draw(mSendTexProces, mTexBounds);
        }
        
    }
    
    if(mFbo->getColorTexture()){
        gl::ScopedMatrices  mat;
        gl::ScopedColor col;
         gl::color(1.0, 1.0, 1.0, 1.0);
        gl::translate(ci::ivec2( 2.0*(getWindowWidth()/3.0), -getWindowHeight()/3.333));
        gl::draw(mFbo->getColorTexture());
    }
    
    //middle block
    {
        gl::ScopedMatrices mat;
        gl::ScopedColor col;
        gl::translate(ci::ivec2( (getWindowWidth()/3.0), getWindowHeight()/5.0));
        gl::color(mCurrentColor);
        gl::drawSolidRect(mTexBounds);
        
        if(mTextTexture){
            gl::translate(ci::vec2(mTexBounds.getWidth()/4.5, mTexBounds.getHeight() ));
            gl::color(0.6, 0.6, 0.6);
            gl::draw(mTextTexture, vec2(0, 0));
        }
        
    }
    
    //moving block
    {
        gl::ScopedColor col;
        gl::ScopedMatrices mat;
        gl::translate(ci::ivec2( 0.0 * (getWindowWidth()/3.0), getWindowHeight()/5.0));
        
        float xAspect =  ((stepDiv.x * mIteraPixel.x ) / (float) mSendTexProces->getWidth() )* mTexBounds.getWidth();
        float yAspect =  ((stepDiv.y * mIteraPixel.y ) / (float) mSendTexProces->getHeight() )* mTexBounds.getHeight();
        
        //iverted aspect ratio
        ci::vec2 aspectInv( (float)mTexBounds.getWidth()/ (float) mSendTexProces->getWidth(), (float)mTexBounds.getHeight()/ (float) mSendTexProces->getHeight() );
        
        gl::translate(ci::vec2(xAspect, yAspect));
        gl::color(0.0, 0.7, 0.8, 0.5);//mCurrentColor);
        gl::drawSolidRect(Rectf(0, 0, stepDiv.x * aspectInv.x, stepDiv.y * aspectInv.y));
    }
    
    
    mParams->draw();
    
}


void ImageSenderApp::processPixels(double currentTime)
{
    
    //process pixels
    if(mSendPixels){
        
        //if incomming msg is true then activate the pixel ready
        {
            
    
            //read incomming message
            string  mLastString;
            uint8_t mInSerial;
            
            // mSerial->writeBytes(const void * data, size_t numBytes)
            if(mReadInMsg && mSerialStarted ){
                try{
                    // read until newline, to a maximum of BUFSIZE bytes
                    
                    //console()<<mSerial->getNumBytesAvailable()<<std::endl;
                    if(mSerial->getNumBytesAvailable() > 0){
                    
                       // mLastString = mSerial->readStringUntil( 's', BUFFER_SIZE );
                        
                        mInSerial = mSerial->readByte();
                        mSerial->flush();
                        mReadInMsg = false;
                        
                        //mSerial->readBytes(void *data, size_t numBytes)
                        
        
                        console()<<"Available: "<<mSerial->getNumBytesAvailable()<<" "<<mInSerial<<std::endl;
                  
                    }
                   
                   
                }
                catch( SerialTimeoutExc &exc ) {
                    CI_LOG_EXCEPTION( "timeout", exc );
                }
            }
            
            
            if(mNexIteration){
                
                
                ////update iteration
                updateIteration();
                
                mCentralPixel = ci::vec2(mIteraPixel) * ci::vec2(stepDiv.x, stepDiv.y) + ci::vec2(stepDiv.x/2.0, stepDiv.y/2.0);
                CI_LOG_V(mCentralPixel);
                mCurrentColor =  mSendImage.getPixel(mCentralPixel);
                CI_LOG_V(mSendImage.getPixel(mCentralPixel));
                
                //https://en.wikipedia.org/wiki/Luma_(video)
                
                //float luma = 0.2126 * mCurrentColor.r + 0.7152 * mCurrentColor.g + 0.0722 * mCurrentColor.b;
                //uint8_t grayValue = floatColorToInt(luma);
                
                uint8_t grayValue = floatColorToInt(mCurrentColor.r );
                uint8_t result[8];
             
                mCurrentByteStr="";
                for(int i = 0; i < 8; ++i) {
                    result[i] = 0 != (grayValue & (1 << i));
                    mCurrentByteStr += to_string(result[i]);
                }
              
                
                //write msg
                if(mSerialStarted){
                    mSendData[0] = grayValue;
                    CI_LOG_V( grayValue << " "<<mCurrentByteStr);
                    mSerial->writeBytes( (uint8_t *)mSendData, NUM_BYTES);
                }
                    
                
                mUpdateRender = true;
                mNexIteration = false;
                mReadInMsg    = true;
            }
            
            
            if(!mReadInMsg){
                if(mInSerial == 's'){
                    mSerialDuratinT = currentTime - mSerialPrevT;
                    
                    mSerialPrevT = currentTime;
                    
                    //time 100 for ms
                    console()<<"Got msg  S "<< mSerialDuratinT * 1000 <<std::endl;
                    
                    //request a Bytes.
                    //mSerial->writeByte('a'); //added
                    mNexIteration = true;
                    
                }
            }
            
            //currentTime
            
           
        }
        

    }
    
}

void ImageSenderApp::renderTexFont()
{
    //render font with new value
    TextLayout simple;
    simple.setFont( mFont );
    simple.setColor( Color( 0.8, 0.8, 0.8f ) );
    
    std::string rgbStr = "("+ to_string(floatColorToInt(mCurrentColor.r))+", "+to_string(floatColorToInt(mCurrentColor.g))+", "+ to_string(floatColorToInt(mCurrentColor.b))+")";
    std::string indexStr = "["+to_string(mIteraPixel.x)+", "+to_string(mIteraPixel.y)+"]";
    
    
    simple.addLine(rgbStr);
    simple.addCenteredLine(indexStr);
    simple.addCenteredLine(mCurrentByteStr);
    mTextTexture = gl::Texture2d::create( simple.render( true, false ) );
    
}

void ImageSenderApp::updateIteration(){
    
    mIteraPixel.x++;
    if(mIteraPixel.x >= mNumPixels.x){
        mIteraPixel.y++;
        mIteraPixel.x = 0;
    }
    
    if(mIteraPixel.y >= mNumPixels.y){
        mFinishSending = true;
        mSendPixels  = 0;
        mIteraPixel = ci::vec2(0, 0);
        CI_LOG_I("DONE SENDING");
    }
    
    console()<<mIteraPixel<<std::endl;
    
}

uint8_t ImageSenderApp::floatColorToInt(float inVal)
{
    union { float f; uint32_t i; } u;
    u.f = 32768.0f + inVal * (255.0f / 256.0f);
    return (uint8_t)u.i;
}


CINDER_APP( ImageSenderApp,  RendererGl( RendererGl::Options().msaa( 16 ) ), [] (App::Settings * settings){
    
    //settings->setHighDensityDisplayEnabled();
})
