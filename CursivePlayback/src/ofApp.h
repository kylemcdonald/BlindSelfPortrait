#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	
	ofPolyline generate(string msg);
	
	ofxXmlSettings xml;	
	vector<ofPolyline> alphabet;
	vector<ofVec2f> offset;
	vector<string> text;
	bool savePdf;
};
