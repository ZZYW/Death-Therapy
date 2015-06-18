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
    Particle(ofVec3f _location);
    
    //Methods
    void update();
    void seek(ofVec3f _target);
    void display();
    ofVec3f getPosition();
    
    //properties
    ofVec3f location;
    ofVec3f velocity;
    ofVec3f acceleration;
    float maxforce;
    float maxspeed;
    bool target_assigned;
    //tail
    //  vector<ofVec3f> history;
    
    
    
};


#endif /* defined(__OculusRenderingBasic__Particle__) */
