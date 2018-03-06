#pragma once

#include "ofMain.h"
#include <json.hpp>

class ofApp : public ofBaseApp{

	public:
                void incrementCursor(int &c, uint8_t &v);
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

                
                int bytesRequired = 12;
                unsigned char bytes[12];
                int bytesRemaining = bytesRequired;
		
};
