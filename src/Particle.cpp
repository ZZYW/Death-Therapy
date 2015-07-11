//
//  Particle.cpp
//  OculusRenderingBasic
//
//  Created by Yang Wang on 6/18/15.
//
//

#include "Particle.h"
Particle::Particle(ofVec3f _location, ofColor c){
    acceleration = ofVec3f(0,0,0);
    velocity = ofVec3f(0,0,0);
    location = _location;
    maxspeed = ofRandom(1000,10000);
    //    maxspeed = 20;
    maxforce = ofRandom(0.04,0.4);
    //    maxforce = 10;
    target_assigned = false;
    color = c;
    flyThreshold = ofRandom(0,100);
}

void Particle::update(){
    velocity += acceleration;
    velocity.limit(maxspeed);
    location += velocity  * ofGetLastFrameTime();
    acceleration = acceleration * 0;
}

void Particle::seek(ofVec3f target){
    ofVec3f desired = target - location;
    desired.normalize();
    desired *= maxspeed;
    ofVec3f steer = desired - velocity;
    steer.limit(maxforce);
    acceleration += steer;
}


void Particle::display(){
    float size = 1;
    ofPushMatrix();
    ofSetColor(255, 255, 255);
    ofDrawBox(location.x, location.y, location.z, size, size, size);
    ofPopMatrix();
}


ofVec3f Particle::getPosition() {
    return location;
}