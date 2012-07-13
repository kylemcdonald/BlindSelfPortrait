#include "testApp.h"

using namespace ofxCv;

int side = 256;

cv::Point findCenter(Mat mat, unsigned char target) {
	int crow = mat.rows / 2, ccol = mat.cols / 2;
	cv::Point bestPoint;
	float bestDistance = numeric_limits<float>::infinity();
	for(int row = 0; row < mat.rows; row++) {
		for(int col = 0; col < mat.cols; col++) {
			if(mat.at<unsigned char>(row, col) == target) {
				float distance = ofDist(crow, ccol, row, col);
				if(distance < bestDistance) {
					bestPoint = cv::Point(col, row);
					bestDistance = distance;
				}
			}
		}
	}
	return bestPoint;
}

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
	centers.resize(n);
}

void testApp::update() {
	float detailSigma = .5;
	float detailScaling = 25 / detailSigma;
	float detailThreshold = 120;
	float maskSigma = 4;//ofClamp(mouseX / 10., .1, 100);
	float maskScaling = 25 / maskSigma;
	float maskThreshold = 105;//mouseY;
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
		centers[i] = findCenter(result[i], 0);
	}
}

void testApp::draw() {
	ofBackground(0);
	
	for(int i = 0; i < original.size(); i++) {
		ofPushMatrix();
		ofSetColor(255);
		drawMat(original[i], 0, 0); ofTranslate(0, side);
		drawMat(detailPass[i], 0, 0); ofTranslate(0, side);
		drawMat(maskPass[i], 0, 0); ofTranslate(0, side);
		drawMat(result[i], 0, 0);
		ofSetColor(ofColor::red);
		ofNoFill();
		ofCircle(toOf(centers[i]), 6);
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
