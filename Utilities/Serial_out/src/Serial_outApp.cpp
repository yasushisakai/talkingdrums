#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class Serial_outApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    char[] getIntensity(Surface *surface);
    
  private:
    int index;
    
};

void Serial_outApp::setup()
{
    index = 0;
    
    
    
}


char[] Serial_outApp::getIntesity(Surface *surface){
    char[] result = new char[];
    
}


void Serial_outApp::mouseDown( MouseEvent event )
{
}

void Serial_outApp::update()
{
}

void Serial_outApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( Serial_outApp, RendererGl )
