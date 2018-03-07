#pragma once

#include "ofMain.h"
#include <json.hpp>

class ofApp : public ofBaseApp{

	public:
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

                unsigned char bytes[16];

                // this is hardcoded!
                const int imageWidth{150};
                const int imageHeight{100};
                ofImage image;
                ofPoint start;
                ofSerial serial;
                int multi{7};
                int cursor{0};
                uint8_t value{0};
                bool hasReceived{false};

                const string checkHead{"L: r="};
                const string base_url{"http://biteater.media.mit.edu/image/send/"};
};
