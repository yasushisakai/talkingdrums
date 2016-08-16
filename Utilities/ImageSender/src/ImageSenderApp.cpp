#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"
#include "cinder/CinderMath.h"
#include "cinder/Log.h"
#include "cinder/params/Params.h"

using namespace ci;
using namespace ci::app;
using namespace std;


const ci::ivec2 windowSize(1280 + 640, 720);

//divide image 20, 20, -> 1280, 720, (64 x 36) =
const ci::ivec2 stepDiv(20, 20);



class ImageSenderApp : public App {
public:
    void setup() override;
    void mouseDown( MouseEvent event ) override;
    void keyDown( KeyEvent event) override;
    void update() override;
    void draw() override;
    
    Surface processPixeletedImage(const Surface input, ci::ivec2 stepAmount, ci::ivec2 & numPixels);
    
private:
    
    ci::ivec2           mDiv;
    ci::Surface         mSendImage;
    ci::Surface         mPixelImage;
    
    gl::Texture2dRef    mSendTex;
    ci::Area            mTexBounds;
    ci::ivec2            mNumPixels;
    
    gl::Texture2dRef    mSendTexProces;
    
    // iterate though the image and send it via serial port
    ci::ivec2           mIteraPixel;
    ci::ColorA          mCurrentColor;
    
    bool                mFinishSending;
    bool                mPixelReady;
    bool                mSendPixels;
    
    //params gui
    params::InterfaceGlRef	mParams;
    float                   mAvgFps;
    
    
};

void ImageSenderApp::setup()
{
    setWindowSize(windowSize);
    
    mSendImage = loadImage(loadAsset("wave.png"));
    
    //process the image
    mPixelImage = processPixeletedImage(mSendImage, stepDiv, mNumPixels);
    
    //copy the surface to GPU Texture
    mSendTex = gl::Texture2d::create(mSendImage);
    mSendTexProces = gl::Texture2d::create(mPixelImage);
    
    
    //calculate height depending on the aspect ratio of the image
    float width = getWindowWidth()/3.0;
    float height =  (width * mSendTex->getHeight() ) /(mSendTex->getWidth());
    
    CI_LOG_I(width<<" "<<height);
    
    
    //intial values
    mTexBounds = ci::Area(0, 0, width, height);
    
    mFinishSending = false;
    mPixelReady    = false;
    
    mIteraPixel = ivec2(0, 0);
    
    //create params
    mParams = params::InterfaceGl::create( getWindow(), "App parameters", toPixels( ivec2( 200, 400 ) ) );
    mParams->addParam("FPS", &mAvgFps, false);
    
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
            CI_LOG_I("START SENDING PIXELS");
            break;
        case 's':
            break;
    }
}


void ImageSenderApp::update()
{
    mAvgFps = getAverageFps();
}

void ImageSenderApp::draw()
{
    gl::clear( Color( 0, 0, 0 ) );
    
    //draw original image
    if(mSendTex){
        gl::ScopedMatrices  mat;
        gl::translate(ci::ivec2(0, getWindowHeight()/5.0));
        gl::draw(mSendTex, mTexBounds);
    }
    
    {
        if(mSendPixels){
            
            
            
            //if incomming msg is true then activate the pixel ready
            {
                if(getElapsedFrames() % 30 == 0){
                    mPixelReady = true;
                    mCurrentColor =  mPixelImage.getPixel(mIteraPixel);
                }
            }
            
            {
                gl::ScopedMatrices mat;
                gl::ScopedColor col;
                gl::translate(ci::ivec2( (getWindowWidth()/3.0), getWindowHeight()/5.0));
                gl::color(mCurrentColor);
                gl::drawSolidRect(mTexBounds);
            }
            
            
            {
                gl::ScopedColor col;
                gl::ScopedMatrices mat;
                gl::translate(ci::ivec2( 2.0 * (getWindowWidth()/3.0), getWindowHeight()/5.0));
                
                
                float xAspect =  ((stepDiv.x * mIteraPixel.x ) / (float) mSendTexProces->getWidth() )* mTexBounds.getWidth();
                float yAspect =  ((stepDiv.y * mIteraPixel.y ) / (float) mSendTexProces->getHeight() )* mTexBounds.getHeight();
                
                //iverted
                ci::vec2 aspectInv( (float)mTexBounds.getWidth()/ (float) mSendTexProces->getWidth(), (float)mTexBounds.getHeight()/ (float) mSendTexProces->getHeight() );
                
                gl::translate(ci::vec2(xAspect, yAspect));
                gl::color(mCurrentColor);
                gl::drawSolidRect(Rectf(0, 0, stepDiv.x * aspectInv.x, stepDiv.y * aspectInv.y));
                
            }
            
            
            //update iteration
            if(mPixelReady){
                
                mIteraPixel.x++;
                if(mIteraPixel.x > mNumPixels.x){
                    mIteraPixel.y++;
                    mIteraPixel.x = 0;
                }
                
                if(mIteraPixel.y > mNumPixels.y){
                    mFinishSending = true;
                    CI_LOG_I("DONE SENDING");
                }
                
                mPixelReady = false;
              //  console()<<mIteraPixel<<std::endl;
            }
            
        }
        
        
    }
    
    //draw pixeleted image, the image that we are going to send
    if(mSendTexProces){
        gl::ScopedBlendAlpha alpha;
        
        gl::ScopedColor col;
        gl::ScopedMatrices  mat;
        gl::translate(ci::ivec2( 2*(getWindowWidth()/3.0), getWindowHeight()/5.0));
        
        //Alpha effect to texture
        if(mSendPixels){
            gl::enableAlphaBlending();
            gl::color(1.0, 1.0, 1.0, 0.5);
        }else{
            gl::color(1.0, 1.0, 1.0, 1.0);
        }
        gl::draw(mSendTexProces, mTexBounds);
    }
    
    
    mParams->draw();
    
}

CINDER_APP( ImageSenderApp,  RendererGl( RendererGl::Options().msaa( 16 ) ), [] (App::Settings * settings){
    
    //settings->setHighDensityDisplayEnabled();
})
