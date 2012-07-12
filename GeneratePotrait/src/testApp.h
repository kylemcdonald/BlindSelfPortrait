#pragma once

#include "ofMain.h"

#include "ofxCv.h"

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	
	vector<cv::Mat> original, lowPass, highPass, highPass8u, sobel;
	cv::Mat sobelX, sobelY, sobelXMag, sobelYMag, sobelDummy;
};
