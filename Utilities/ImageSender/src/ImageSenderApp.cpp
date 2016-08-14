#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ImageSenderApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void ImageSenderApp::setup()
{
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
}

CINDER_APP( ImageSenderApp, RendererGl )
