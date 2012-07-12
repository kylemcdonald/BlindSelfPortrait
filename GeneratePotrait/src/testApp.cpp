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
	
	detailPass.resize(n);
	maskPass.resize(n);
	result.resize(n);
}

void testApp::update() {
	float detailSigma = .5;
	float detailScaling = 25 / detailSigma;
	float detailThreshold = 128;
	float maskSigma = 6.;//ofClamp(mouseX / 10., .1, 100);
	float maskScaling = 25 / maskSigma;
	float maskThreshold = 110.;//mouseX;
	for(int i = 0; i < original.size(); i++) {
		cv::GaussianBlur(original[i], lowPass, cv::Size(), detailSigma, detailSigma);
		highPass = original[i] - lowPass;
		highPass.convertTo(highPass8uc3, CV_8UC3, detailScaling * 128, 128);
		convertColor(highPass8uc3, detailPass[i], CV_RGB2GRAY);
		threshold(detailPass[i], detailThreshold);
		
		cv::GaussianBlur(original[i], lowPass, cv::Size(), maskSigma, maskSigma);
		highPass = original[i] - lowPass;
		highPass.convertTo(highPass8uc3, CV_8UC3, maskScaling * 128, 128);
		convertColor(highPass8uc3, maskPass[i], CV_RGB2GRAY);
		threshold(maskPass[i], maskThreshold);
		
		result[i] = maskPass[i] | detailPass[i];
	}
}

void testApp::draw() {
	ofBackground(0);
	
	for(int i = 0; i < original.size(); i++) {
		ofPushMatrix();
		drawMat(original[i], 0, 0); ofTranslate(0, side);
		//drawMat(lowPass[i], 0, 0); ofTranslate(0, side);
		//drawMat(highPass[i], 0, 0); ofTranslate(0, side);
		//drawMat(highPass8uc3[i], 0, 0); ofTranslate(0, side);
		drawMat(detailPass[i], 0, 0); ofTranslate(0, side);
		drawMat(maskPass[i], 0, 0); ofTranslate(0, side);
		drawMat(result[i], 0, 0); ofTranslate(0, side);
		ofPopMatrix();
		ofTranslate(side, 0);
		
		if(ofGetKeyPressed()) {
			ofImage img;
			copy(result[i], img);
			img.saveImage("result-" + ofToString(i) + ".png");
		}
	}
	
	ofDrawBitmapStringHighlight(ofToString(mouseX) + "," + ofToString(mouseY), 10, 20);
}
