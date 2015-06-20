#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofBackground(0);
	ofSetLogLevel( OF_LOG_VERBOSE );
	ofSetVerticalSync( true );
	showOverlay = false;
	predictive = true;
//	ofHideCursor();
	oculusRift.baseCamera = &cam;
    //oculusRift.setup();
	
	cam.begin();
	cam.end();
    
    kinect.setRegistration(true);
    kinect.init();
    kinect.open();
    
    oscReceiver.setup(PORT);
    
    gui = new ofxUICanvas();
    gui->addSlider("BACKGROUND", 0.0, 255.0,100.0);
    gui->addSlider("MELLOWREADING", 0.0, 1.0, 0.0);

    gui->autoSizeToFitWidgets();
    ofAddListener(gui->newGUIEvent, this, &ofApp::guiEvent);
    gui->loadSettings("settings.xml");
}


//--------------------------------------------------------------
void ofApp::update()
{
    kinect.update();
    //OSC
    while(oscReceiver.hasWaitingMessages()){
        ofxOscMessage m;
        oscReceiver.getNextMessage(&m);
        if(m.getAddress() =="/muse/elements/experimental/mellow"){
            mellowReading = m.getArgAsFloat(0);
            cout<<mellowReading<<endl;
        }
    }
}
//--------------------------------------------------------------
void ofApp::draw()
{
	
	if(oculusRift.isSetup()){
		
		if(showOverlay){
			
			oculusRift.beginOverlay(-230, 320,240);
			ofRectangle overlayRect = oculusRift.getOverlayRectangle();
			
			ofPushStyle();
			ofEnableAlphaBlending();
			ofFill();
			ofSetColor(255, 40, 10, 200);
			
			ofRect(overlayRect);
			
			ofSetColor(255,255);
			ofFill();
			ofDrawBitmapString("ofxOculusRift by\nAndreas Muller\nJames George\nJason Walters\nElie Zananiri\nFPS:"+ofToString(ofGetFrameRate())+"\nPredictive Tracking " + (oculusRift.getUsePredictiveOrientation() ? "YES" : "NO"), 40, 40);
            
            ofSetColor(0, 255, 0);
            ofNoFill();
            ofCircle(overlayRect.getCenter(), 20);
			
			ofPopStyle();
			oculusRift.endOverlay();
		}
        
        ofSetColor(255);
		glEnable(GL_DEPTH_TEST);


		oculusRift.beginLeftEye();
		drawScene();
		oculusRift.endLeftEye();
		
		oculusRift.beginRightEye();
		drawScene();
		oculusRift.endRightEye();
		
		oculusRift.draw();
		
		glDisable(GL_DEPTH_TEST);
    }
	else{
		cam.begin();
		drawScene();
		cam.end();
	}
	
}

const int Step = 2;//50;//2;//20;//2;
const int Width = 640;
const int Height = 480;
const int XCellCount = Width / Step;
const int YCellCount = Height / Step;
bool bInitCellsOnce = true;
int frameCounter = 0;
const int KinectCount = 1;//2;
ofVec3f oldPoints[XCellCount][YCellCount][KinectCount];
//TODO: add some per point noise data for concentration??


//--------------------------------------------------------------
void ofApp::drawScene()
{
    ofPushMatrix();
    //drawPointCloud();
    for(int kinectIndex = 0; kinectIndex < KinectCount; kinectIndex++)
    {
        drawPointCloud(kinectIndex);
    }
    ofPopMatrix();
}

//-------------------------------------------------------------


ofVec3f randomWalk(ofVec3f before, float scalar)
{
    ofVec3f noise = ofVec3f(
        ofNoise(ofGetElapsedTimef() + before.x),
        ofNoise(ofGetElapsedTimef() + before.y),
        ofNoise(ofGetElapsedTimef() + before.z));
    noise = (noise - 0.5f) * 2.0f; //[0,1] --> [-1,+1]
    noise = noise.getNormalized();
    
    float moveAmount =50.0f *scalar;//20.0f * scalar;//100.0f * scalar;// 0.1f; //HACK
    before += noise * moveAmount;
    return before;
}

ofVec3f moveToward(ofVec3f before, ofVec3f destination)
{
    ofVec3f delta = destination - before;
    //if (delta.squareDistance(ofVec3f(0,0,0)) < 100.0f)
    {
    //    return destination;
    }
    
    ofVec3f approachAmount = 0.25f * delta;//10.0f * delta.getNormalized(); //HACK
    before += approachAmount;
    return before;
}

void ofApp::drawPointCloud(int kinectIndex)
{
    //int kinectIndex = 0; //TODO: pull kinect 1 in some cases!
    int w = Width;//640;
    int h = Height;//480;
    ofMesh mesh;
    mesh.setMode(OF_PRIMITIVE_TRIANGLES);
    //mesh.setMode(OF_PRIMITIVE_LINES);
    //OF_PRIMITIVE_LINE_LOOP);//OF_PRIMITIVE_TRIANGLES);//OF_PRIMITIVE_POINTS);
    int step = Step;//2;//10;
    const float MellowThreshold = 0.95f;//0.5f;
    
    bool bFocused = (mellowReading > MellowThreshold);//0.5f);
    /*if (bFocused)
    {
        printf("FOCUSED!\n");
    }
    else
    {
        printf("NOT FOCUSED!\n");
    }*/
    
    
    if (frameCounter  < 100)
    {
        frameCounter++;
        bInitCellsOnce = true;
    }
    else
    {
        bInitCellsOnce = false;
    }
    
    int index = 0;
    for(int y = 0, iy=0; y < h; y += step, iy++) {
        for(int x = 0, ix=0; x < w; x += step, ix++) {
            //if(kinect.getDistanceAt(x, y) > 0 && kinect.getDistanceAt(x, y) < 1400)
            {
                ofVec3f focusedPoint = kinect.getWorldCoordinateAt(x, y);
                ofVec3f oldPoint = oldPoints[ix][iy][kinectIndex];
                
                if (bInitCellsOnce)
                {//make sure we give good data on the first iteration!
                    oldPoints[ix][iy][kinectIndex] = focusedPoint;
                    oldPoint = focusedPoint;
                }
                ofVec3f newPoint = oldPoint;
                //if (lostConcentrationWithVariation(x,y, concentration, thresholdFromImage))
                //if (true)
                if (!bFocused)
                {
                    float concentrationError = 1.0 - (mellowReading * (1.0f / MellowThreshold)); //[0, 0.5] --> [1, 0]
                    float maxDist = concentrationError * 250.0f;//100.0f;//2000.0f;//1.0f;//10.0f;
                    
                    newPoint = randomWalk(oldPoint, concentrationError);
                    //if (false)
                    {
                        float dist2 = (newPoint).squareDistance(focusedPoint);
                        //if (dist2 <= maxDist * maxDist)
                        if (dist2 >= maxDist * maxDist)
                        {
                            ofVec3f delta = (newPoint - focusedPoint).getNormalized() * maxDist;
                            newPoint = focusedPoint + delta;
                            //newPoint = focusedPoint + (focusedPoint - newPoint).getNormalized() * maxDist;
                        }
                    }
                }
                else
                {
                    //newPoint = focusedPoint;
                    //newPoint = (newPoint + focusedPoint) / 2.0f;
                    newPoint = moveToward(oldPoint, focusedPoint);
                }
                //newPoint = focusedPoint;
                
                //if((kinect.getDistanceAt(x, y) > 0 )
                if((kinect.getDistanceAt(x, y) > 0 ) && (kinect.getDistanceAt(x, y) < 1400))
                {
                    //mesh.addVertex(newPoint);
                    float dr = 2;//0;//20;//2; //similar to pointsize
                    ofVec3f dVertex[4] = {
                        ofVec3f(-dr, -dr, 0),
                        ofVec3f(+dr, -dr, 0),
                        
                        ofVec3f(-dr, +dr, 0),
                        ofVec3f(+dr, +dr, 0)
                        
                    };
                    /*
                    for(int i=0; i<2*3*2; i++)
                    {
                        mesh.addColor(kinect.getColorAt(x,y));
                    }
                    */
                    
                    std::vector<ofVec3f> corners;
                    for (int i=0; i<4; i++)
                    {
                        mesh.addColor(kinect.getColorAt(x,y));
                        //corners.push_back(newPoint + dVertex[i]);
                        mesh.addVertex(newPoint + dVertex[i]);
                    }
                    /*
                    mesh.addVertex(corners[0]);
                    mesh.addVertex(corners[1]);
                    mesh.addVertex(corners[1]);
                    mesh.addVertex(corners[2]);
                    mesh.addVertex(corners[2]);
                    mesh.addVertex(corners[0]);
                    
                    mesh.addVertex(corners[1]);
                    mesh.addVertex(corners[3]);
                    mesh.addVertex(corners[3]);
                    mesh.addVertex(corners[2]);
                    mesh.addVertex(corners[2]);
                    mesh.addVertex(corners[1]);
                    */
                    mesh.addTriangle(index+0, index+1, index+2);
                    //mesh.add
                    mesh.addTriangle(index+1, index+3, index+2);
                    index += 4;
                }
                
                oldPoints[ix][iy][kinectIndex] = newPoint;
                //setOldPointAt(x,y,newPoint);
                //mesh.addVertex(kinect.getWorldCoordinateAt(x, y));

                
            }
        }
    }
    
    bInitCellsOnce = false;
    
    glPointSize(2);
    ofPushMatrix();
    // the projected points are 'upside down' and 'backwards'
    ofScale(1, -1, -1);
    ofTranslate(0, 0, -1000); // center the points a bit
    ofEnableDepthTest();
    //mesh.drawVertices();
    mesh.drawFaces();//drawTriangles();
    ofDisableDepthTest();
    ofPopMatrix();
}
//--------------------------------------------------------------

void ofApp::guiEvent(ofxUIEventArgs &e)
{
    if(e.getName()=="BACKGROUND"){
        ofxUISlider *slider = e.getSlider();
        ofBackground(slider->getScaledValue());
    }
    if(e.getName() == "MELLOWREADING"){
        ofxUISlider *slider = e.getSlider();
        mellowReading = slider->getScaledValue();
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	if( key == 'f' )
	{
		//gotta toggle full screen for it to be right
		ofToggleFullscreen();
	}
	
	if(key == 's'){
		oculusRift.reloadShader();
	}
	
	if(key == 'l'){
		oculusRift.lockView = !oculusRift.lockView;
	}
	
	if(key == 'o'){
		showOverlay = !showOverlay;
	}
	if(key == 'r'){
		oculusRift.reset();
		
	}
	if(key == 'h'){
		ofHideCursor();
	}
	if(key == 'H'){
		ofShowCursor();
	}
	
	if(key == 'p'){
		predictive = !predictive;
		oculusRift.setUsePredictedOrientation(predictive);
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{

}

void ofApp::exit()
{
    gui->saveSettings("settings.xml");
    delete gui;
}
//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y)
{
 //   cursor2D.set(x, y, cursor2D.z);
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
//    cursor2D.set(x, y, cursor2D.z);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{

}
