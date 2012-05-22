#pragma once

#include "ofMain.h"

struct bySecond {
	template <class First, class Second>
	bool operator()(std::pair<First, Second> const &a, std::pair<First, Second> const &b) { 
		return a.second < b.second;
	}
};

class City {
public:
	City(int x, int y)
	: x(x)
	, y(y)
	, group(0)
	, connected(false)
	, neighbor(NULL) {
	}
	float distance(City& city) const {
		int dx = x - city.x, dy = y - city.y;
		return sqrtf(dx * dx + dy * dy);
	}
	operator ofVec2f () {
		return ofVec2f(x, y);
	}
	bool getConnected() {
		return connected;
	}
	int getGroup() const {
		return group;
	}
	void setGroup(int group) {
		this->group = group;
		connected = true;
	}
protected:
	int x, y;
	int group;
	City *neighbor;
	bool connected;
};

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	
	int groupCount;
	bool propagate(int x, int y);
	
	int w, h;
	ofImage img;
	vector<City*> all;
	vector<City*> grid;
	
	ofMesh connections;
};
