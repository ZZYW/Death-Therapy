//
//  Particle.h
//  OculusRenderingBasic
//
//  Created by Yang Wang on 6/18/15.
//
//

#ifndef __OculusRenderingBasic__Particle__
#define __OculusRenderingBasic__Particle__

#include <stdio.h>
#include <iostream>
#include "ofMain.h"

class Particle {
public:
    
    //constructor
    Particle(ofVec3f _location, ofColor _c);
    
    //Methods
    void update();
    void seek(ofVec3f _target);
    void display();
    ofVec3f getPosition();
    
    //properties
    ofVec3f location;
    ofVec3f velocity;
    ofVec3f acceleration;
    ofColor color;
    float maxforce;
    float maxspeed;
    bool target_assigned;
    float flyThreshold;
    //tail
    //  vector<ofVec3f> history;
    
    
    
};


#endif /* defined(__OculusRenderingBasic__Particle__) */
