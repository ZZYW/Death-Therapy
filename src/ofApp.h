#pragma once

#include "ofMain.h"
#include "ofxUI.h"
#include "ofxOpenCv.h"
#include "ofxOculusDK2.h"
#include "ofxKinect.h"
#include "ofxOsc.h"
#include "Particle.h"


#define PORT 4444
#define NUM_MSG_STRING 20
//#define USE_TWO_KINECTS


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
    void exit();
	
	void drawScene();
    void drawPointCloud();//(int kinectIndex);
    void guiEvent(ofxUIEventArgs &e);
//    void audioOut(float * input, int bufferSize, int nChannels);
	
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
#ifdef USE_TWO_KINECTS
    ofxKinect kinect2;
#endif
    ofxKinect kinect;

    
    ofxOscReceiver oscReceiver;
    float sensorReading;
    
    
    ofxUICanvas *gui;
    
    
    //sound
    ofSoundStream noiseSoundStream;
    
    float userFreq;
    float userPwm;
    
    float freq;
    float pwm;
    float phase;
    
    vector<float> buf;
    float noiseSeed;
    
//    vector<float> individualVariety;
    
    ofSoundPlayer leftSound;
    ofSoundPlayer rightSound;

    
    bool hideGUI;
    
    
    
    
};
