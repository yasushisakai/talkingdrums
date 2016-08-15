#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"
#include "cinder/CinderMath.h"
#include "cinder/Log.h"

using namespace ci;
using namespace ci::app;
using namespace std;


const ci::ivec2 windowSize(1280, 720);

//divide image 20, 20, -> 1280, 720, (64 x 36) =
const ci::ivec2 stepDiv(20, 20);


class ImageSenderApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    Surface processPixeletedImage(const Surface input, ci::ivec2 stepAmount);
    
private:
    
    ci::ivec2           mDiv;
    ci::Surface         mSendImage;
    ci::Surface         mPixelImage;
    
    gl::Texture2dRef    mSendTex;
    ci::Area            mTexBounds;
    
    gl::Texture2dRef    mSendTexProces;
    
};

void ImageSenderApp::setup()
{
    setWindowSize(windowSize);
 
    mSendImage = loadImage(loadAsset("wave.png"));
    
    //process the image
    mPixelImage = processPixeletedImage(mSendImage, stepDiv);

    //copy the surface to GPU Texture
    mSendTex = gl::Texture2d::create(mSendImage);
    mSendTexProces = gl::Texture2d::create(mPixelImage);
    
    
    //calculate height depending on the aspect ratio of the image
    float width = getWindowWidth()/2.0;
    float height =  (width * mSendTex->getHeight() ) /(mSendTex->getWidth());
    
    console()<<width<<" "<<height<<std::endl;
    
    mTexBounds = ci::Area(0, 0, width, height);
}

Surface ImageSenderApp::processPixeletedImage(const Surface input, ci::ivec2 stepAmount)
{
    
    ci::Area areaProcess = input.getBounds();
    Surface pixelImage =  input.clone( areaProcess );
    
    //initial values
    int counterPix = 0;
    ci::ivec2 centerDiv( stepAmount.x / 2.0, stepAmount.y /2.0);
    ci::ivec2 xyIter(centerDiv.x, centerDiv.y);
    ci::ivec2 imageSize(areaProcess.x2, areaProcess.y2);
    
    ci::vec2 numPixels = imageSize/stepAmount;
    
    console()<<"Center "<< centerDiv<<std::endl;
    console()<<numPixels<<" "<<numPixels.x * numPixels.y<<std::endl;
    console()<<"Area "<<imageSize.x<<" "<<imageSize.y<<std::endl;
    
    //process the Image
    ci::ColorA col = mSendImage.getPixel(xyIter);
    
    for(int i = 0; i < imageSize.x; i++){
        for(int j = 0; j < imageSize.y; j++){
            
            //change color depending ifon the center pixel
            if(j % stepAmount.y == 0){
                counterPix++;
                
                xyIter.y += stepAmount.y;
                col = input.getPixel(xyIter);
                
                //console()<<i<<" "<< j <<" "<<counterPix<<std::endl;
                //console()<<"center "<<xyIter<<std::endl;
            }
            
            
            pixelImage.setPixel(ci::ivec2(i, j), col);
            
        }
        
        xyIter.y = (int)centerDiv.y;
        
        if(i %  stepAmount.x == 0){
            xyIter.x += stepAmount.x;
            //console()<<xyIter<<std::endl;
        }
        
    }

    return pixelImage;
}


void ImageSenderApp::mouseDown( MouseEvent event )
{
}


void ImageSenderApp::update()
{
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
    
    //draw pixeleted image, the image that we are going to send
    if(mSendTexProces){
        gl::ScopedMatrices  mat;
        gl::translate(ci::ivec2(getWindowWidth()/2.0, getWindowHeight()/5.0));
        gl::draw(mSendTexProces, mTexBounds);
    }
}

CINDER_APP( ImageSenderApp,  RendererGl( RendererGl::Options().msaa( 16 ) ), [] (App::Settings * settings){
    
})
