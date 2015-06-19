#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxOculusDK2.h"
#include "ofxKinect.h"
#include "ofxOsc.h"

#define PORT 12344
#define NUM_MSG_STRING 20



struct particleTarget {
    ofVec3f location;
    bool isChosen;
};

class ofApp : public ofBaseApp
{
  public:
	
	void setup();
	void update();
	void draw();
	
	void drawScene();
    void drawPointCloud();
	
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	ofxOculusDK2		oculusRift;

	ofLight				light;
	ofEasyCam			cam;
	bool showOverlay;
	bool predictive;
    
    ofVec3f cursor2D;
    ofVec3f cursor3D;
    
    ofVec3f cursorRift;
    ofVec3f demoRift;
    
    ofVec3f cursorGaze;
    
    ofxKinect kinect;

    
    ofxOscReceiver oscReceiver;
    float mellowReading;
    
    
    
    
};
