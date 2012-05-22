#include "ofApp.h"

#include "ofxCv.h"

bool ofApp::propagate(int x, int y) {
	bool propagated = false;
	if(x > 0 && x < w && y > 0 && y < h) {
		int i = y * w + x;
		if(grid[i] && !grid[i]->getConnected()) {
			grid[i]->setGroup(groupCount);
			propagate(x-1,y-1);
			propagate(x+0,y-1);
			propagate(x+1,y-1);
			propagate(x-1,y+0);
			propagate(x+1,y+0);
			propagate(x-1,y+1);
			propagate(x+0,y+1);
			propagate(x+1,y+1);
			propagated = true;
		}
	}
	return propagated;
}

void ofApp::setup() {
	ofSetVerticalSync(true);
	
	img.loadImage("img.png");
	img.setImageType(OF_IMAGE_GRAYSCALE);
	ofxCv::threshold(img, 128); // screenshot was blurry
	
	w = img.getWidth();
	h = img.getHeight();
	int n = w * h;
	
	grid.resize(n, NULL);
	
	int i = 0;
	for(int y = 0; y < h; y++) {
		for(int x = 0; x < w; x++) {
			if(img.getColor(x, y).getBrightness() > 128) {
				City* cur = new City(x, y);
				all.push_back(cur);
				grid[i] = cur;
			}
			i++;
		}
	}
	
	groupCount = 0;
	for(int y = 0; y < h; y++) {
		for(int x = 0; x < w; x++) {
			if(propagate(x, y)) {
				groupCount++;
			}
		}
	}
	
	cout << "total groups: " << groupCount << endl;
	
	// draw paired vs remaining
	img.setImageType(OF_IMAGE_COLOR);
	i = 0;
	for(int y = 0; y < h; y++) {
		for(int x = 0; x < w; x++) {
			if(grid[i]) {
				ofColor cur;
				cur.setHsb((grid[i]->getGroup() * 32) % 255, 255, 128);
				img.setColor(x, y, cur);
			}
			i++;
		}
	}
	img.update();
	
	connections.setMode(OF_PRIMITIVE_LINES);
}

void ofApp::update() {
	
}

void ofApp::draw() {
	ofBackground(0);
	ofSetColor(255);
	ofTranslate((int) (ofGetWidth() - img.getWidth()) / 2, (int) (ofGetHeight() - img.getHeight()) / 2);
	img.draw(0, 0);
	if(ofGetKeyPressed()) {
		ofTranslate(.5, .5);
		connections.draw();
	}
}