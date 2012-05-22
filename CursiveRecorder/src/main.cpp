#include "ofApp.h"
#include "ofAppGlutWindow.h"

int main() {
	ofAppGlutWindow window;
	ofSetupOpenGL(&window, 972, 828, OF_WINDOW);
	ofRunApp(new ofApp());
}
