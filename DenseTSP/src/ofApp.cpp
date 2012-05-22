#include "ofApp.h"

#include "ofxCv.h"

void thin(ofPixels& img) {
	int w = img.getWidth(), h = img.getHeight();
	int ia1=-w-1,ia2=-w-0,ia3=-w+1,ib1=-0-1,ib3=-0+1,ic1=+w-1,ic2=+w-0,ic3=+w+1;
	unsigned char* p = img.getPixels();
	vector<unsigned int> q;
	for(int y = 1; y + 1 < h; y++) {
		for(int x = 1; x + 1 < w; x++) {
			int i = y * w + x;
			if(p[i]) {
				q.push_back(i);
			}
		}
	}
	int n = q.size();	
	for(int i=0;i<n;i++){int j=q[i];if(!p[j+ia1]&&!p[j+ia2]&&!p[j+ia3]&&p[j+ic1]&&p[j+ic2]&&p[j+ic3]){p[j]=0;}}
	for(int i=0;i<n;i++){int j=q[i];if(!p[j+ia3]&&!p[j+ib3]&&!p[j+ic3]&&p[j+ia1]&&p[j+ib1]&&p[j+ic1]){p[j]=0;}}
	for(int i=0;i<n;i++){int j=q[i];if(!p[j+ic1]&&!p[j+ic2]&&!p[j+ic3]&&p[j+ia1]&&p[j+ia2]&&p[j+ia3]){p[j]=0;}}
	for(int i=0;i<n;i++){int j=q[i];if(!p[j+ia1]&&!p[j+ib1]&&!p[j+ic1]&&p[j+ia3]&&p[j+ib3]&&p[j+ic3]){p[j]=0;}}
	for(int i=0;i<n;i++){int j=q[i];if(!p[j+ia2]&&!p[j+ia3]&&!p[j+ib3]&&p[j+ib1]&&p[j+ic2]){p[j]=0;}}
	for(int i=0;i<n;i++){int j=q[i];if(!p[j+ib3]&&!p[j+ic3]&&!p[j+ic2]&&p[j+ib1]&&p[j+ia2]){p[j]=0;}}
	for(int i=0;i<n;i++){int j=q[i];if(!p[j+ic2]&&!p[j+ic1]&&!p[j+ib1]&&p[j+ia2]&&p[j+ib3]){p[j]=0;}}
	for(int i=0;i<n;i++){int j=q[i];if(!p[j+ib1]&&!p[j+ia1]&&!p[j+ia2]&&p[j+ic2]&&p[j+ib3]){p[j]=0;}}
}

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
	
	img.loadImage("c.png");
	img.setImageType(OF_IMAGE_GRAYSCALE);
	ofxCv::threshold(img, 128); // screenshot was blurry
	thin(img.getPixelsRef()); // make lines don't have adjacent neighbors
	removeIslands(img.getPixelsRef()); // remove pixels with no neighbors
	
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
	
	// we're not looking for a genuine TSP
	// kind of. the preference is to travel the same cities
	// so here's an algorithm:
	
	// 1 start with one group
	// 2 draw all the pixels in the group, backtracking if necessary
	// 3 find the nearest un-drawn group and backtrack to the closest point
	// 4 go to 2 for the new group until there are no more groups left
	
	// second pass connects remaining points
	// every assignment should reduce the number of groups
	/*
	 vector<City*> remaining;
	 for(int i = 0; i < all.size(); i++) {
	 if(!all[i]->getConnected()) {
	 remaining.push_back(all[i]);
	 }
	 }
	 
	 cout << "remaining size: " << remaining.size() << endl;
	 
	 // build NxM distance matrix
	 n = remaining.size();
	 typedef std::pair<int, int> MatchPair;
	 typedef std::pair<MatchPair, float> MatchDistancePair;
	 vector<MatchDistancePair> allMatches;
	 for(int i = 0; i < n; i++) {
	 for(int j = i + 1; j < n; j++) {
	 float curDistance = remaining[i]->distance(*remaining[j]);
	 allMatches.push_back(MatchDistancePair(MatchPair(i, j), curDistance));
	 }
	 }
	 
	 // sort all possible matches by distance
	 sort(allMatches.begin(), allMatches.end(), bySecond());
	 
	 vector<bool> matchedObjects(n, false);
	 for(int k = 0; k < allMatches.size(); k++) {
	 MatchPair& match = allMatches[k].first;
	 int i = match.first;
	 int j = match.second;
	 // only use match if both objects are unmatched, lastSeen is set to 0
	 if(!matchedObjects[i] && !matchedObjects[j] && !remaining[i]->matches(*remaining[j])) {
	 matchedObjects[i] = true;
	 matchedObjects[j] = true;
	 remaining[i]->connect(*remaining[j]);
	 connections.addVertex((ofVec2f) *remaining[i]);
	 connections.addVertex((ofVec2f) *remaining[j]);
	 }
	 }
	 */
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