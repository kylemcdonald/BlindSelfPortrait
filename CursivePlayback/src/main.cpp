#include "ofApp.h"
#include "ofAppGlutWindow.h"

int main() {
	ofAppGlutWindow window;
	window.setGlutDisplayString("rgba double samples>=8 depth");
	ofSetupOpenGL(&window, 1024, 1024, OF_WINDOW);
	ofRunApp(new ofApp());
}
