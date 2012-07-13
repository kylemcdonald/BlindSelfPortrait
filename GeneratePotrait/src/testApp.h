#pragma once

#include "ofMain.h"

#include "ofxCv.h"

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	
	vector<cv::Mat> original, detailPass, maskPass, result;
	vector<cv::Point> centers;
	
	cv::Mat lowPass, highPass, highPass8uc3;
};
