#pragma once

#include "ofMain.h"

#include "ofxCv.h"
#include "ofxOsc.h"
#include "ofxPathfinder.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	
	void keyPressed(int key);
	
	int w, h;
	vector<bool> original;
	typedef std::pair<int, int> intPair;
	list<intPair> remaining;
	ofImage img;
	
	ofMesh connections;
	ofPolyline target, drawn;
	ofEasyCam cam;
	
	float progress;
	int lastState;
	
	ofxOscSender osc;
	
	ofxPathfinder pathfinder;
};
