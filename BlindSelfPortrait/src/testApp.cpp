// x find the face
// x crop to an area around the face
// ignore the background somehow, using a white bg or background subtraction, or whatever?
// x find relevant points using canny edge detection
// x generate points with the pathfinder addon using EraserLine code
// while the person is closing their eyes, send points to the machine

#include "testApp.h"

using namespace ofxCv;

float pointsPerPixel = .5;
float pointsPerSecond = 100;
float squareResize = 2.5;
float innerSquareResize = .8;
float eyeThreshold = 3.0;
float backgroundBlur = 64;
float backgroundAdapt = 240; // seconds
float cannyParam1 = 900;
float cannyParam2 = 600;
string ip = "127.0.0.1";
int port = 10000;

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

ofRectangle intersect(const ofRectangle& a, const ofRectangle& b) {
	ofRectangle c;
	c.x = MAX(a.x, b.x);
	c.y = MAX(a.y, b.y);
	float ar = a.x + a.width, br = b.x + b.width;
	float ab = a.y + a.height, bb = b.y + b.height;
	c.width = MIN(ar, br) - c.x;
	c.height = MIN(ab, bb) - c.y;
	return c;
}

ofRectangle resizeFromCenter(ofRectangle rect, float scale) {
	rect.x += rect.width / 2;
	rect.y += rect.height / 2;
	rect.width *= scale;
	rect.height *= scale;
	rect.x -= rect.width / 2;
	rect.y -= rect.height / 2;
	return rect;
}

ofRectangle getCenteredSquare(ofRectangle& rect) {
	ofRectangle out;
	out.x = rect.x + rect.width / 2;
	out.y = rect.y + rect.height / 2;
	out.width = out.height = MIN(rect.width, rect.height);
	out.x -= out.width / 2;
	out.y -= out.height / 2;
	return out;
}

void testApp::setup() {
	ofSetVerticalSync(true);
	
	cam.initGrabber(1280, 720);
	cameraBox = ofRectangle(0, 0, 1280, 720);
	
	tracker.setup();
	tracker.setRescale(.25);
	
	background.setLearningTime(backgroundAdapt);
	background.setThresholdValue(10);
	
	square.allocate(256, 256, OF_IMAGE_COLOR);
	imitate(gray, square, CV_8UC1);
	imitate(edge, gray);
	imitate(fg, gray);
	
	osc.setup(ip, port);
	ofxOscMessage msg;
	msg.setAddress("/velocity");
	msg.addFloatArg(350);
	osc.sendMessage(msg);
	
	moving = false;
	lastMoving = false;
	
	beep.loadSound("beep-2.wav");
}

void testApp::update() {
	lastMoving = moving;
	//moving = ofGetKeyPressed(' ');
	
	cam.update();
	if(cam.isFrameNew()) {
		Mat camMat = toCv(cam);
		tracker.update(camMat);
		//background.update(cam, thresholded);
		
		//blur(thresholded, backgroundBlur);
		//threshold(thresholded, 128);
		//thresholded.update();
		
#ifdef MANUAL_OVERRIDE
		moving = ofGetKeyPressed(OF_KEY_RETURN);
#else
		moving = false;
#endif
		if(tracker.getFound()) {
			eyeOpenness = tracker.getGesture(ofxFaceTracker::LEFT_EYE_OPENNESS);
#ifndef MANUAL_OVERRIDE
			moving = (eyeOpenness < eyeThreshold);
#endif
			
			if(!moving) {
				boundingBox = tracker.getImageFeature(ofxFaceTracker::FACE_OUTLINE).getBoundingBox();
				
				innerBox = resizeFromCenter(boundingBox, innerSquareResize);
				innerBox = intersect(innerBox, cameraBox);
				innerBox = getCenteredSquare(innerBox);
				
				boundingBox = resizeFromCenter(boundingBox, squareResize);
				boundingBox = intersect(boundingBox, cameraBox);
				boundingBox = getCenteredSquare(boundingBox);
				
				Mat roiMat(camMat, toCv(boundingBox));
				resize(roiMat, square);
				square.update();
				
				//Mat thresholdedRoiMat = Mat(toCv(thresholded), toCv(boundingBox));
				//resize(thresholdedRoiMat, fg);
				//fg.update();
				
				convertColor(square, gray, CV_RGB2GRAY);
				Canny(gray, edge, cannyParam1 * 2, cannyParam2 * 2, 5);	
				//bitwise_and(edge, fg, edge);
				thin(edge);
				removeIslands(edge.getPixelsRef());
				lighten(edge.getPixelsRef(), 5);
				edge.update();
			}
		}
	}
	
	if(moving) {
		if(!lastMoving) {
			pathfinder.setup(edge);
			updatePath();
		}
		progress += ofGetLastFrameTime();
	}
	
	float state = progress * pointsPerSecond;
	int curState = (int) state;
	if(curState < target.size()) {
		cleanedUp = false;
		if(curState == 0 || lastState < curState) {
			while(lastState < curState) {
				ofVec2f cur = target[lastState];
				drawn.addVertex(cur);
				ofxOscMessage msg;
				msg.setAddress("/move");
				msg.addFloatArg(ofNormalize(cur.x, 0, square.getWidth()));
				msg.addFloatArg(ofNormalize(cur.y, 0, square.getHeight()));
				osc.sendMessage(msg);
				lastState++;
			}
		}
	} else {
		if(!cleanedUp) {
			//origin();
			beep.play();
			cleanedUp = true;
		}
	}
}

void testApp::updatePath() {
	progress = 0;
	lastState = 0;
	
	int w = edge.getWidth(), h = edge.getHeight();
	int n = w * h;
	
	original.resize(n, false);
	
	int i = 0;
	int lastx, lasty;
	for(int y = 0; y < h; y++) {
		for(int x = 0; x < w; x++) {
			if(edge.getColor(x, y).getBrightness() > 128) {
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
	target.clear();
	drawn.clear();
	while(!remaining.empty()) {
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
				target.addVertex(ofVec2f(x, y));
			}
			lastx = x, lasty = y;
		}
		remaining.erase(nearIt);
	}
	
	target = target.getResampledBySpacing(1. / pointsPerPixel);
	//target = target.getSmoothed(8);
}

void testApp::draw() {
	ofBackground(0);
	
	ofTranslate((ofGetWidth() - cam.getWidth()) / 2, (ofGetHeight() - cam.getHeight()) / 2);
	
	ofSetColor(255);
	cam.draw(0, 0);
	tracker.draw();
	
	square.draw(square.width * 0, 0);
	//fg.draw(square.width * 1, 0);
	edge.draw(square.width * 1, 0);
	
	ofSetColor(ofColor::red);
	target.draw();
	ofSetColor(255);
	drawn.draw();
	
	ofNoFill();
	ofRect(boundingBox);
	ofRect(innerBox);
	
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofSetColor(64);
	//thresholded.draw(0, 0);
	ofEnableBlendMode(OF_BLENDMODE_ALPHA); 
	
	ofSetColor(255);
	ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, 20);
	ofDrawBitmapString(ofToString(eyeOpenness), 10, 40);
}

void testApp::mouseDragged(int x, int y, int button) {
	cannyParam1 = x;
	cannyParam2 = y;
}

void testApp::keyPressed(int key) {
	if(key == 'f') {
		ofToggleFullscreen();
	}
	if(key == 'r') {
		background.reset();
	}
	if(key == 'o') {
		origin();
	}
}

void testApp::keyReleased(int key) {
	if(key == OF_KEY_RETURN) {
		square.saveImage(ofToString(ofGetSystemTime()) + ".png");
	}
}	

void testApp::origin() {
	ofxOscMessage msg;
	msg.setAddress("/move");
	msg.addFloatArg(.5);
	msg.addFloatArg(.5);
	osc.sendMessage(msg);
}