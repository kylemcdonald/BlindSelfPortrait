#include "ofApp.h"

ofRectangle getScreenBoundingBox() {
	return ofRectangle(0, 0, ofGetWidth(), ofGetHeight());
}

void glMap(const ofRectangle& from, const ofRectangle& to) {
	ofTranslate(to.x, to.y);
	ofScale(to.width, to.height);
	ofScale(1. / from.width, 1. / from.height);
	ofTranslate(-from.x, -from.y);
}

void glFit(const ofRectangle& from, const ofRectangle& to) {
	ofRectangle fromFit = from;
	if(from.width / to.width > from.height / to.height) {
		fromFit.height = fromFit.width * (to.height / to.width);
	} else {
		fromFit.width = fromFit.height * (to.width / to.height);
	}
	glMap(fromFit, to);
}

void ofApp::setup() {
	ofSetVerticalSync(true);
	ofSetFrameRate(60);
	
	xml.loadFile("all.xml");
	
	for(int i = 0; i < xml.getNumTags("character"); i++) {
		ofPolyline polyline;
		ofVec2f p;
		
		xml.pushTag("character", i);
		int n = xml.getNumTags("p");
		for(int j = 0; j < n; j++) {
			stringstream strm(xml.getValue("p", "", j));
			strm >> p;
			polyline.addVertex(p);
		}
		xml.popTag();
		
		polyline = polyline.getResampledBySpacing(8);
		alphabet.push_back(polyline);
		offset.push_back(polyline.getVertices().back());
	}
	
	text = ofSplitString(ofBufferFromFile("text.txt"), "\n");
	
	savePdf = false;
}

ofPolyline ofApp::generate(string msg) {
	ofPolyline polyline;
	ofVec2f base, line;
	ofStringReplace(msg, "\t", "    ");
	for(int i = 0; i < msg.size(); i++) {
		if(msg[i] == '\n') {
			line.y += offset[' '].x;
			base = line;
		} else {
			int j = msg[i] - ' ';
			ofPolyline& cur = alphabet[j];
			for(int k = 0; k < cur.size(); k++) {
				polyline.addVertex(base + cur[k]);
			}
			base += offset[j];
		}
	}
	polyline = polyline.getSmoothed(ofMap(mouseY, 0, ofGetHeight(), 0, 128));
	polyline = polyline.getResampledBySpacing(ofMap(mouseX, 0, ofGetWidth(), 1, 50, true));
	return polyline;
}

void ofApp::update() {
	
}

void ofApp::draw() {
	if(savePdf) {
		ofBeginSaveScreenAsPDF("out.pdf");
	}
	
	ofBackground(255);
	
	ofEnableAlphaBlending();
	ofSetLineWidth(1);
	ofEnableSmoothing();
	
	ofPushMatrix();
	
	int n = text.size();
	
	ofSetColor(0);
	ofRectangle screen = getScreenBoundingBox();
	screen.height /= n;
	
	for(int i = 0; i < n; i++) {
		ofPolyline polyline = generate(text[i]);
		
		ofPushMatrix();
		glFit(polyline.getBoundingBox(), screen);
		polyline.draw();
		ofPopMatrix();
		ofTranslate(0, ofGetHeight() / n);
	}
	ofPopMatrix();
	
	if(savePdf) {
		ofEndSaveScreenAsPDF();
		savePdf = false;
	}
}

void ofApp::keyPressed(int key) {
	if(key == 's') {
		savePdf = true;
	}
}