#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class SerialCommunicationApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void SerialCommunicationApp::setup()
{
}

void SerialCommunicationApp::mouseDown( MouseEvent event )
{
}

void SerialCommunicationApp::update()
{
}

void SerialCommunicationApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( SerialCommunicationApp, RendererGl )
