#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

void removeIslands(ofPixels& img) {
	int w = img.getWidth(), h = img.getHeight();
	int ia1=-w-1,ia2=-w-0,ia3=-w+1,ib1=-0-1,ib3=-0+1,ic1=+w-1,ic2=+w-0,ic3=+w+1;
	unsigned char* p = img.getPixels();
	for(int y = 1; y + 1 < h; y++) {
		for(int x = 1; x + 1 < w; x++) {
			int i = y * w + x;
			if(p[i]) {
				if(!p[i+ia1]&&!p[i+ia2]&&!p[i+ia3]&&!p[i+ib1]&&!p[i+ib3]&&!p[i+ic1]&&!p[i+ic2]&&!p[i+ic3]) {
					p[i] = 0;
				}
			}
		}
	}
}

void lighten(ofPixels& img, int offset) {
	int w = img.getWidth(), h = img.getHeight();
	unsigned char* p = img.getPixels();
	for(int y = 0; y < h; y++) {
		for(int x = 0; x < w; x++) {
			int i = y * w + x;
			if(p[i] == 0) {
				p[i] = offset;
			}
		}
	}
}

float pointsPerPixel = .5;
float pointsPerSecond = 50;
string ip = "192.168.1.162";
int port = 10000;

void ofApp::setup() {
	ofSetVerticalSync(true);
	
	progress = 0;
	lastState = 0;
	osc.setup(ip, port);
	
	img.loadImage("img.png");
	img.setImageType(OF_IMAGE_GRAYSCALE);
	ofxCv::threshold(img, 128); // screenshot was blurry
	thin(img); // make lines don't have adjacent neighbors
	removeIslands(img.getPixelsRef()); // remove pixels with no neighbors
	
	lighten(img.getPixelsRef(), 25);
	pathfinder.setup(img);
	img.update();
	
	w = img.getWidth();
	h = img.getHeight();
	int n = w * h;
	
	original.resize(n, false);
	
	int i = 0;
	int lastx, lasty;
	for(int y = 0; y < h; y++) {
		for(int x = 0; x < w; x++) {
			if(img.getColor(x, y).getBrightness() > 128) {
				original[i] = true;
				remaining.push_back(intPair(x, y));
				lastx = x;
				lasty = y;
			}
			i++;
		}
	}
	
	// 1 determine the closest unvisited neighbor
	// 2 do an a* search to find the shortest path (white costs less than black)
	// 3 walk one pixel in that direction
	// 4 add the current vertex if it is not along a straight line
	// 5 go to 1 
	
	int x = lastx, y = lasty;
	i = 0;
	while(!remaining.empty()) {
		//connections.addColor(ofColor::fromHsb(i++ % 255, 255, 255));
		int nearDistance = 0;
		list<intPair>::iterator nearIt, it;
		for(it = remaining.begin(); it != remaining.end(); it++) {
			intPair& cur = *it;
			int xd = x - cur.first;
			int yd = y - cur.second;
			int distance = xd * xd + yd * yd;
			if(it == remaining.begin() || distance < nearDistance) {
				nearIt = it;
				nearDistance = distance;
			}
		}
		intPair& next = *nearIt;
		x = next.first, y = next.second;
		pathfinder.find(x, y, lastx, lasty);
		ofPolyline& path = pathfinder.path;
		for(int j = 0; j < path.size(); j++) {
			x = path[j].x, y = path[j].y;
			if(lastx != x && lasty != y) { // simplify
				connections.addVertex(ofVec3f(x, y, i++));
				target.addVertex(ofVec2f(x, y));
			}
			lastx = x, lasty = y;
		}
		remaining.erase(nearIt);
	}
	
	target = target.getResampledBySpacing(1. / pointsPerPixel);
	//target = target.getSmoothed(8);
}

void ofApp::update() {
	if(ofGetKeyPressed(' ')) {
		progress += ofGetLastFrameTime();
	}
	
	float state = progress * pointsPerSecond;
	int curState = (int) state;
	if(curState < target.size()) {
		if(curState == 0 || lastState < curState) {
			while(lastState < curState) {
				ofVec2f cur = target[lastState];
				drawn.addVertex(cur);
				ofxOscMessage msg;
				msg.setAddress("/move");
				msg.addFloatArg(ofNormalize(cur.x, 0, ofGetWidth()));
				msg.addFloatArg(ofNormalize(cur.y, 0, ofGetHeight()));
				osc.sendMessage(msg);
				lastState++;
			}
		}
	}
}

void ofApp::draw() {
	ofBackground(0);
	ofSetColor(255);
	ofTranslate((int) (ofGetWidth() - img.getWidth()) / 2, (int) (ofGetHeight() - img.getHeight()) / 2);
	if(ofGetKeyPressed('3')) {
		cam.begin();
		ofScale(1, -1, .1);
		ofTranslate((int) (-img.getWidth()) / 2, (int) (-img.getHeight()) / 2);
		ofSetColor(255);
		connections.setMode(OF_PRIMITIVE_LINE_STRIP);
		connections.draw();
		glPointSize(3);
		ofSetColor(ofColor::red);
		connections.setMode(OF_PRIMITIVE_POINTS);
		connections.draw();
		cam.end();
	} else {
		ofSetColor(255);
		img.draw(0, 0);
		ofSetLineWidth(1);
		target.draw();
		ofSetLineWidth(3);
		ofSetColor(ofColor::red);
		drawn.draw();
	}
}

void ofApp::keyPressed(int key) {
	if(key == 'r') {
		progress = 0;
		drawn.clear();
		lastState = 0;
	}
}