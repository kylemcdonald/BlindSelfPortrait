#include "testApp.h"

using namespace ofxCv;

int side = 256;

// could be optimized by searching out from the center and returning first result
cv::Point findClosest(Mat mat, int x, int y, unsigned char target) {
	cv::Point bestPoint(x, y);
	float bestDistance = numeric_limits<float>::infinity();
	for(int row = 0; row < mat.rows; row++) {
		for(int col = 0; col < mat.cols; col++) {
			if(mat.at<unsigned char>(row, col) == target) {
				float distance = ofDistSquared(x, y, row, col);
				if(distance < bestDistance) {
					bestPoint = cv::Point(col, row);
					bestDistance = distance;
				}
			}
		}
	}
	return bestPoint;
}

cv::Point findCenter(Mat mat, unsigned char target) {
	return findClosest(mat, mat.cols / 2, mat.rows / 2, target);
}

cv::Point findRandom(Mat mat, unsigned char target) {
	return findClosest(mat, ofRandom(0, mat.cols), ofRandom(0, mat.rows), target);
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
	remaining.resize(n);
	targets.resize(n);
	paths.resize(n);
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

		bitwise_not(result[i], terrain);
		unsigned char brightness = ofClamp(mouseX, 1, 250);
		terrain -= brightness;
		terrain += brightness;
		toOf(terrain, terrainImage);
		pathfinder.setup(terrainImage);
		
		targets[i].clear();
		paths[i].clear();
		
		/*
		// solve path through given landmarks
		targets[i].push_back(findCenter(terrain, 255));
		for(int j = 0; j < 12; j++) {
			targets[i].push_back(findRandom(terrain, 255));
		}
		for(int j = 0; j < targets[i].size() - 1; j++) {
			cv::Point source = targets[i][j], target = targets[i][j + 1];
			pathfinder.find(source.x, source.y, target.x, target.y);
			paths[i].addVertexes(pathfinder.path.getVertices());
		}
		*/
		
		/*
		// trying to make a first step, then jump ahead a little
		cv::Point first = findCenter(terrain, 255);
		cv::Point second = findRandom(terrain, 255);
		pathfinder.find(first.x, first.y, second.x, second.y);
		ofPolyline cur = pathfinder.path.getResampledByCount(32).getSmoothed(4);
		targets[i].push_back(first);
		targets[i].push_back(second);
		
		ofVec2f final = cur.getVertices()[cur.size() - 1];
		ofVec2f prev = cur.getVertices()[cur.size() - 2];
		ofVec2f direction = (final - prev).getNormalized();
		direction *= 16;
		targets[i].push_back(toCv(final + direction));
		
		paths[i] = cur;
		*/
		
		/*
		// mark the ones you've already seen, try to maintain consistency
		remaining[i] = terrain.clone();
		ofVec2f direction(8, 0);
		cv::Point cur = findCenter(terrain, 255), next;
		for(int j = 0; j < 16; j++) {
			remaining[i].at<unsigned char>(cur.y, cur.x) = 0;
			next = findClosest(remaining[i], cur.x + direction.x, cur.y + direction.y, 255);
			pathfinder.find(cur.x, cur.y, next.x, next.y);
			ofPolyline& path = pathfinder.path;
			paths[i].addVertexes(path.getVertices());
			direction = toOf(next) - toOf(cur);
			direction.normalize();
			direction *= 8;
			
			vector<cv::Point> pti;
			vector<vector<cv::Point> > pts;
			Mat(toCv(path)).copyTo(pti);
			pts.push_back(pti);
			polylines(remaining[i], pts, false, cv::Scalar(0), 4);
			
			targets[i].push_back(next);
			
			cur = next;
		}
		*/
		
		/*
		// always go for the next closest, mark where you've been
		cv::Point first = findCenter(terrain, 255), second;
		targets[i].push_back(first);
		remaining[i] = terrain.clone();		
		for(int j = 0; j < 64; j++) {
			second = findClosest(remaining[i], first.x, first.y, 255);
			pathfinder.find(first.x, first.y, second.x, second.y);
			ofPolyline& cur = pathfinder.path;
			for(int k = 0; k < cur.size(); k++) {
				remaining[i].at<unsigned char>(cur[k].y, cur[k].x) = 0;
			}
			paths[i].addVertexes(cur.getVertices());
			first = second;
		}
		*/
		
		/*
		// mark the path you've seen, pick at random
		remaining[i] = terrain.clone();
		cv::Point cur = findCenter(terrain, 255), next;
		for(int j = 0; j < 128; j++) {
			float range = 8;
			cv::Point target(cur.x + ofRandom(-range, +range), cur.y + ofRandom(-range, +range));
			next = findClosest(remaining[i], target.x, target.y, 255);
			pathfinder.find(cur.x, cur.y, next.x, next.y);
			ofPolyline& path = pathfinder.path;
			paths[i].addVertexes(path.getVertices());
			
			vector<cv::Point> pti;
			vector<vector<cv::Point> > pts;
			Mat(toCv(path)).copyTo(pti);
			pts.push_back(pti);
			polylines(remaining[i], pts, false, cv::Scalar(0), 8);
			
			targets[i].push_back(next);
			
			cur = next;
		}
		invert(remaining[i]);
		threshold(remaining[i], 128);
		*/
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
		//drawMat(remaining[i], 0, 0); ofTranslate(0, side);
		drawMat(result[i], 0, 0); ofTranslate(0, side);
		//ofSetLineWidth(1);
		ofSetColor(cyanPrint);
		ofNoFill();		
		for(int j = 0; j < targets[i].size(); j++) {
			//ofCircle(toOf(targets[i][j]), 6);
		}
		ofSetColor(magentaPrint);
		paths[i].draw();
		
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
