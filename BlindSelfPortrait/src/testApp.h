#pragma once

#include "ofMain.h"

#include "ofxCv.h"
#include "ofxFaceTracker.h"
#include "ofxPathfinder.h"
#include "ofxOsc.h"

#define MANUAL_OVERRIDE

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void mouseDragged(int x, int y, int button);
	void keyPressed(int key);
	void keyReleased(int key);
	void updatePath();
	void origin();
	
	ofVideoGrabber cam;
	ofxFaceTracker tracker;
	ofRectangle cameraBox, boundingBox, innerBox;
	
	ofxCv::RunningBackground background;
	ofImage thresholded;
	
	ofImage square, gray, edge;
	ofImage fg;
	
	ofxPathfinder pathfinder;
	vector<bool> original;
	typedef std::pair<int, int> intPair;
	list<intPair> remaining;
	ofPolyline target;
	
	ofxOscSender osc;
	float progress;
	int lastState;
	ofPolyline drawn;
	
	bool moving, lastMoving;
	float eyeOpenness;
	
	ofSoundPlayer beep;
	bool cleanedUp;
};
