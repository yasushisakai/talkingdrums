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


const ci::ivec2 windowSize(1280 + 640, 720);

//divide image 20, 20, -> 1280, 720, (64 x 36) =
const ci::ivec2 stepDiv(20, 20);

const int BAU_RATE = 9600;


class ImageSenderApp : public App {
public:
    void setup() override;
    void mouseDown( MouseEvent event ) override;
    void keyDown( KeyEvent event) override;
    void update() override;
    void draw() override;
    
    Surface processPixeletedImage(const Surface input, ci::ivec2 stepAmount, ci::ivec2 & numPixels);
    void processPixels();
    
    void initFBO();
    void renderOutputImage();
    
    //PORT
    void initPort();
    
private:
    
    ci::Surface         mSendImage;
    ci::Surface         mPixelImage;
    
    gl::Texture2dRef    mSendTex;
    ci::Area            mTexBounds;
    ci::ivec2           mNumPixels;
    
    gl::Texture2dRef    mSendTexProces;
    
    
    //FBO for the output test image
    gl::FboRef			mFbo;
    
    
    // iterate though the image and send it via serial port
    ci::ivec2           mIteraPixel;
    ci::ColorA          mCurrentColor;
    
    bool                mFinishSending;
    bool                mPixelReady;
    bool                mSendPixels;
    bool                mDrawOriginal;
    
    //fonts
    gl::Texture2dRef	mTextTexture;
    Font                mFont;
    
    //Serial
    SerialRef           mSerial;
    
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
}

void ImageSenderApp::setup()
{
    setWindowSize(windowSize);
    
    mSendImage = loadImage(loadAsset("wave.png"));
    
    //FBO
    initFBO();
    
    //PORT
    initPort();
    
    //process the image
    mPixelImage = processPixeletedImage(mSendImage, stepDiv, mNumPixels);
    
    //copy the surface to GPU Texture
    mSendTex = gl::Texture2d::create(mSendImage);
    mSendTexProces = gl::Texture2d::create(mPixelImage);
    
    
    //calculate height depending on the aspect ratio of the image
    float width = getWindowWidth()/3.0;
    float height =  (width * mSendTex->getHeight() ) /(mSendTex->getWidth());
    
    CI_LOG_I(width<<" "<<height);
    
    //create font
    mFont =  Font( "Arial", 20 );
    
    //intial values
    mTexBounds = ci::Area(0, 0, width, height);
    
    mFinishSending = false;
    mPixelReady    = false;
    
    mDrawOriginal  = false;
    
    mIteraPixel    = ivec2(0, 0);
    
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
    ci::ColorA col = mSendImage.getPixel(xyIter);
    
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


void ImageSenderApp::mouseDown( MouseEvent event )
{
    
}

void ImageSenderApp:: keyDown( KeyEvent event)
{
    switch(event.getChar()){
        case 'a':
            mSendPixels = true;
            mCurrentColor =  mPixelImage.getPixel(ci::ivec2(0, 0));
            CI_LOG_I("START SENDING PIXELS");
            break;
        case 's':
            break;
    }
}


////----
void ImageSenderApp::update()
{
    mAvgFps = getAverageFps();
    
    renderOutputImage();
    
}

void ImageSenderApp::renderOutputImage()
{
    if(mSendPixels){
        gl::ScopedFramebuffer fbScp( mFbo );
        
        gl::ScopedViewport scpVp( ivec2( 0 ), mTexBounds.getSize() );
        gl::ScopedMatrices matrices;
        gl::setMatricesWindow( mTexBounds.getSize(), true);
        gl::setModelMatrix(ci::mat4());
        
        gl::ScopedColor col;
        gl::ScopedMatrices mat;
        
        float xAspect =  ((stepDiv.x * mIteraPixel.x ) / (float) mSendTexProces->getWidth() )* mTexBounds.getWidth();
        float yAspect =  ((stepDiv.y * mIteraPixel.y ) / (float) mSendTexProces->getHeight() )* mTexBounds.getHeight();
        
        //iverted aspect ratio
        ci::vec2 aspectInv( (float)mTexBounds.getWidth()/ (float) mSendTexProces->getWidth(), (float)mTexBounds.getHeight()/ (float) mSendTexProces->getHeight() );
        
        gl::translate(0, 0);
        gl::translate(ci::vec2(xAspect, yAspect));
        gl::color(mCurrentColor);
        gl::drawSolidRect(Rectf(0, 0, stepDiv.x * aspectInv.x, stepDiv.y * aspectInv.y));
    }
}

////----
void ImageSenderApp::draw()
{
    gl::clear( Color( 0, 0, 0 ) );
    
    
    processPixels();
    
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
                gl::color(1.0, 1.0, 1.0, 0.5);
            }else{
                gl::color(1.0, 1.0, 1.0, 1.0);
            }
            gl::draw(mSendTexProces, mTexBounds);
        }
        
    }
    
    if(mFbo->getColorTexture()){
        gl::ScopedMatrices  mat;
        gl::translate(ci::ivec2( 2.0*(getWindowWidth()/3.0), -getWindowHeight()/3.333));
        gl::draw(mFbo->getColorTexture());
    }
    
    
    mParams->draw();
    
}


void ImageSenderApp::processPixels()
{
    
    //process pixels
    if(mSendPixels){
        
        //if incomming msg is true then activate the pixel ready
        {
            if(getElapsedFrames() % 5 == 0){
                mPixelReady = true;
                ci::ivec2 centrPixel = mIteraPixel * stepDiv;
                mCurrentColor =  mPixelImage.getPixel(centrPixel);
                
                //render font with new value
                TextLayout simple;
                simple.setFont( mFont );
                simple.setColor( Color( 0.7, 0.7, 0.7f ) );
                
                
                std::string rgbStr = "("+ to_string(mCurrentColor.r)+", "+to_string(mCurrentColor.g)+", "+ to_string(mCurrentColor.b)+")";
                std::string indexStr = "["+to_string(mIteraPixel.x)+", "+to_string(mIteraPixel.y)+"]";
                
                uint8_t grayValue = ceil(int(mCurrentColor.r * 255.0));
                
                uint8_t result[8];
                std::string byteStr;
                for(int i = 0; i < 8; ++i) {
                    result[i] = 0 != (grayValue & (1 << i));
                    byteStr += to_string(result[i]);
                }
                
                simple.addLine(rgbStr);
                simple.addCenteredLine(indexStr);
                simple.addCenteredLine(byteStr);
                mTextTexture = gl::Texture2d::create( simple.render( true, false ) );
                
            }
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
            gl::color(mCurrentColor);
            gl::drawSolidRect(Rectf(0, 0, stepDiv.x * aspectInv.x, stepDiv.y * aspectInv.y));
        }
        
        
        //update iteration
        if(mPixelReady){
            
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
            
            mPixelReady = false;
            console()<<mIteraPixel<<std::endl;
        }
    }
    
}

CINDER_APP( ImageSenderApp,  RendererGl( RendererGl::Options().msaa( 16 ) ), [] (App::Settings * settings){
    
    //settings->setHighDensityDisplayEnabled();
})