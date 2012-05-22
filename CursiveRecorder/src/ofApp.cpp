#include "ofApp.h"

void ofApp::setup() {
	ofSetVerticalSync(true);
	cursive.loadImage("cursive.jpg");
	curLetter = 'a';
	mesh.setMode(OF_PRIMITIVE_LINE_STRIP);
}

void ofApp::update() {
	
}

void ofApp::draw() {
	float scale = ofGetWidth() / cursive.getWidth();
	ofPushMatrix();
	ofScale(scale, scale);
	ofSetColor(255);
	cursive.draw(0, 0);
	ofPopMatrix();r 
	
	ofSetColor(ofColor::red);
	mesh.draw();
}

void ofApp::mousePressed(int x, int y, int button) {
	xml.addTag("character");
	int which = xml.getNumTags("character") - 1;
	string name;
	name += curLetter;
	xml.setAttribute("character", "name", name, which);
	xml.pushTag("character", which);
	
	mesh.clear();
}

void ofApp::mouseDragged(int x, int y, int button) {
	mesh.addVertex(ofVec2f(x, y));
}

void ofApp::mouseReleased(int x, int y, int button) {
	ofVec2f base = mesh.getVertex(0);
	for(int i = 0; i < mesh.getNumVertices(); i++) {
		xml.addTag("p");
		xml.setValue("p", ofToString(ofVec2f(mesh.getVertex(i) - base)), i);
	}

	xml.popTag();
	curLetter++;
	xml.saveFile("out.xml");
}