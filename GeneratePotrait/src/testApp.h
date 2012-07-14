#pragma once

#include "ofMain.h"

#include "ofxCv.h"
#include "ofxPathfinder.h"

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	
	vector<cv::Mat> original, detailPass, maskPass, remaining, result;
	vector< vector<cv::Point> > targets;
	
	cv::Mat lowPass, highPass, highPass8uc3;
	
	ofxPathfinder pathfinder;
	cv::Mat terrain;
	ofPixels terrainImage;
	vector<ofPolyline> paths;
};
