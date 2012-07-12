#include "testApp.h"

using namespace ofxCv;

int side = 256;

void testApp::setup() {
	ofSetVerticalSync(true);
	
	ofDirectory dir;
	dir.listDir("faces");
	int n = MIN(6, dir.size());
	for(int i = 0; i < n; i++) {
		ofImage cur;
		cur.loadImage(dir.getPath(i));
		Mat curMat;
		copy(cur, curMat, CV_32F);
		original.push_back(curMat);
	}
	
	lowPass.resize(n);
	highPass.resize(n);
	highPass8u.resize(n);
	sobel.resize(n);
}

void testApp::update() {
	float radius = 100;//MIN(200, mouseX);
	float scaling = 2;//MIN(100, mouseY);
	int sobelRadius = -1; // scharr
	float sobelScale = .1;
	for(int i = 0; i < original.size(); i++) {
		ofxCv::blur(original[i], lowPass[i], radius);
		highPass[i] = original[i] - lowPass[i];
		
		cv::Sobel(highPass[i], sobelX, CV_32F, 1, 0, sobelRadius, sobelScale, .5);
		cv::Sobel(highPass[i], sobelY, CV_32F, 0, 1, sobelRadius, sobelScale, .5);
		convertColor(sobelX, sobelXMag, CV_RGB2GRAY);
		convertColor(sobelY, sobelYMag, CV_RGB2GRAY);
		imitate(sobelDummy, sobelXMag);
		sobelDummy.setTo(cv::Scalar(.5));
		vector<Mat> sobelChannels;
		sobelChannels.push_back(sobelXMag);
		sobelChannels.push_back(sobelYMag);
		sobelChannels.push_back(sobelDummy);
		cv::merge(sobelChannels, sobel[i]);
		
		highPass[i].convertTo(highPass8u[i], CV_8UC3, scaling * 128, 128);
	}
}

void testApp::draw() {
	ofBackground(0);
	
	for(int i = 0; i < original.size(); i++) {
		ofPushMatrix();
		drawMat(original[i], 0, 0); ofTranslate(0, side);
		//drawMat(lowPass[i], 0, 0); ofTranslate(0, side);
		//drawMat(highPass[i], 0, 0); ofTranslate(0, side);
		drawMat(highPass8u[i], 0, 0); if(!ofGetKeyPressed()) ofTranslate(0, side);
		drawMat(sobel[i], 0, 0); ofTranslate(0, side);
		ofPopMatrix();
		ofTranslate(side, 0);
		
		if(ofGetKeyPressed()) {
			ofImage img;
			copy(sobel[i], img);
			img.saveImage("sobel-" + ofToString(i) + ".png");
		}
	}
}
