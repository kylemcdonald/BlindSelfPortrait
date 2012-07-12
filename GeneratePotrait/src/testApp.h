#pragma once

#include "ofMain.h"

#include "ofxCv.h"

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	
	vector<cv::Mat> original;
	vector<cv::Mat> detailPass, maskPass, result;
	
	cv::Mat lowPass, highPass, highPass8uc3;
};
