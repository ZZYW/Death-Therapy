#include "ofApp.h"

//--------------------------------------------------------------

int bufSize = 512;
int sampleRate = 44100;
float volume = 1;
bool receiveOscData;

void ofApp::setup()
{
	ofBackground(255);
    ofEnableSmoothing();
	ofSetLogLevel( OF_LOG_VERBOSE );
	ofSetVerticalSync( true );
	predictive = true;
//	ofHideCursor();
	oculusRift.baseCamera = &cam;
//    oculusRift.setup();
	
	cam.begin();
	cam.end();
    
    kinect.setRegistration(true);
    kinect.init();
    kinect.open();
    
#ifdef USE_TWO_KINECTS
    kinect2.init();
    kinect2.open();
#endif
    
    oscReceiver.setup(PORT);
    
//    userFreq = 100.0;
//    userPwm = 0.5;
//    noiseSeed = 0.0;
//    freq = userFreq;
//    pwm = userPwm;
//    phase = 0.0;
//    buf.resize(bufSize);
//    noiseSoundStream.setup(this, 1, 0, sampleRate, bufSize, 4);
    
    
    gui = new ofxUICanvas();
    gui->setTheme(5);
    gui->setColorBack(ofColor(255));
    gui->addFPS();
    
    gui->addSpacer();
    gui->addLabel("EEG Reading");
    gui->addSlider("MELLOW READING", 0.0, 1.0, &mellowReading);
    gui->addToggle("RECEIVE OSC DATA", true);
    
    gui->addSpacer();
    gui->addLabel("CAMERA ADJUSTMENT");
    gui->addSlider("CAMERA X", -1000.0, 1000.0,0.0);
    gui->addSlider("CAMERA Y",-1000.0,1000.0,0.0);
    gui->addSlider("CAMERA Z", -1000.0, 1000.0, 0.0);
    
    gui->addSpacer();
    gui->addLabel("KINECT ADJUSTMENT");
    gui->addSlider("KINECT TWO: X", -1000.0, 1000.0, 0.0);
    gui->addSlider("KINECT TWO: Y", -1000.0, 1000.0, 0.0);
    gui->addSlider("KINECT TWO: Z", -1000.0, 1000.0, 0.0);

//    gui->addSlider("KINECT SEPERATION", -1000.0, 1000.0, 0.0);
    gui->addSlider("QUAD SIZE",1.0,10.0,2.0);
    
    gui->autoSizeToFitWidgets();
    ofAddListener(gui->newGUIEvent, this, &ofApp::guiEvent);
    gui->loadSettings("settings.xml");
    
}


//--------------------------------------------------------------
float cameraX = 522;
float cameraY = 100;
float cameraZ = -500;

void ofApp::update()
{
    kinect.update();
#ifdef USE_TWO_KINECTS
    kinect2.update();
#endif
    //OSC
    if(receiveOscData){
        while(oscReceiver.hasWaitingMessages()){
            ofxOscMessage m;
            oscReceiver.getNextMessage(&m);
            if(m.getAddress() =="/muse/elements/experimental/mellow"){
                mellowReading = m.getArgAsFloat(0);
            }
        }
    }
    
    //Cam Position
//    cam.setPosition(cameraX, cameraY, cameraZ);
//    cout<<"Camera Position: X->"<<cam.getPosition().x<<"  Y->"<<cam.getPosition().y<<"  Z->"<<cam.getPosition().z<<endl;
    
    
    //Sound
    userFreq = ofMap(mellowReading, 0, 1, 1, 2000);
    userPwm = ofMap(mellowReading, 0, 1, 0, 1);
    
}
//--------------------------------------------------------------
void ofApp::draw()
{
	
	if(oculusRift.isSetup()){
        
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

float quadSize = 2;
const int Step = 2;//50;//2;//20;//2;
const int Width = 640;
const int Height = 480;
const int XCellCount = Width / Step;
const int YCellCount = Height / Step;
bool bInitCellsOnce = true;
int frameCounter = 0;
#ifdef USE_TWO_KINECTS
    const int KinectCount = 2;
#else
    const int KinectCount = 1;//2;
#endif
ofVec3f oldPoints[XCellCount][YCellCount][KinectCount];
//TODO: add some per point noise data for concentration??
float Kinect2X = 0;
float Kinect2Y = 0;
float Kinect2Z = 0;


//--------------------------------------------------------------
/*
void ofApp::audioOut(float * output, int bufferSize, int nChannels){
    for (int i=0; i<bufferSize; i++) {
        //freq smoothly reaches userFreq
        freq += ( userFreq - freq ) * 0.001;
        //pwm smoothly reaches userPwm
        pwm += ( userPwm - pwm ) * 0.001;
        
        //Change phase, and push it into [0, 1] range
        phase += freq / sampleRate;
        phase = fmodf( phase, 1.0 );
        
        //Calculate the output audio sample value
        //Instead of 1 and 0 we use 1 and -1 output values
        //for the sound wave to be symmetrical along y-axe
        float v = ( phase < pwm ) ? 1.0 : -1.0;
        
        noiseSeed+=0.01;
        
        v = ofMap(ofNoise(noiseSeed), 0, 1, -1.0, 1.0);
        
        //Set the computed value to the left and the right
        //channels of output buffer,
        //also using global volume value defined above
        output[ i*2 ] = output[ i*2 + 1 ] = v * volume; //but i still don't understand?!
        //        output[i]=v*volume;
        //        cout<<i<<endl;
        //why the hell use i * 2 ?? and why use two equals??
        
        //Set the value to buffer buf, used for rendering
        //on the screen
        //Note: bufferSize can occasionally differ from bufSize
        if ( i < bufSize ) {
            buf[ i ] = v;
        }
    }
}
 */

//--------------------------------------------------------------
void ofApp::drawScene()
{
    
    ofPushMatrix();
    //drawPointCloud();
    for(int kinectIndex = 0; kinectIndex < KinectCount; kinectIndex++)
    {
        ofPushMatrix();
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
        drawPointCloud(kinectIndex);
        ofDisableBlendMode();
        ofPopMatrix();
    }
    ofPopMatrix();
}

//-------------------------------------------------------------

ofVec3f randomWalk(ofVec3f before, float scalar, float mellowReading)
{
//    ofVec3f noise = ofVec3f(
//        ofNoise(ofGetElapsedTimef() + before.x),
//        ofNoise(ofGetElapsedTimef() + before.y),
//        ofNoise(ofGetElapsedTimef() + before.z));
//    
//    noise = (noise - 0.5f) * 2.0f; //[0,1] --> [-1,+1]
//    
    float noiseStep = 2 * ofGetLastFrameTime();
//    cout<<noiseStep<<endl;
    ofVec3f noise = ofVec3f(ofRandom(-noiseStep,noiseStep),ofRandom(-noiseStep,noiseStep),ofRandom(-noiseStep,noiseStep));
    noise = noise.getNormalized();
    
    float multiplier = ofMap(mellowReading, 0, 1, 5, 15);
    float moveAmount = multiplier *scalar;//20.0f * scalar;//100.0f * scalar;// 0.1f; //HACK
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
    if (kinectIndex == 1) //the second kinect
    {
        ofTranslate(Kinect2X,Kinect2Y, Kinect2Z);
//        ofTranslate(0,0,0);
        ofRotateY(180);
    }
    
    
    //int kinectIndex = 0; //TODO: pull kinect 1 in some cases!
    int w = Width;//640;
    int h = Height;//480;
    ofMesh mesh;
//    mesh.setMode(OF_PRIMITIVE_POINTS);
    mesh.setMode(OF_PRIMITIVE_TRIANGLES);
//    mesh.setMode(OF_PRIMITIVE_LINE_LOOP);
    
    int step = Step;//2;//10;
    const float MellowThreshold = 0.95f;//0.5f;
    
    bool bFocused = (mellowReading > MellowThreshold);//0.5f);
    
    if (frameCounter  < 100)
    {
        frameCounter++;
        bInitCellsOnce = true;
    }
    else
    {
        bInitCellsOnce = false;
    }
    
    ofxKinect *usingKinect = &kinect;
    #ifdef USE_TWO_KINECTS
    if (kinectIndex == 1)
    {
        usingKinect = &kinect2;
    }
    #endif
    
    int index = 0;
    for(int y = 0, iy=0; y < h; y += step, iy++) {
        for(int x = 0, ix=0; x < w; x += step, ix++) {
            if(kinect.getDistanceAt(x, y) > 0 && kinect.getDistanceAt(x, y) < 1400)
            {
                ofVec3f focusedPoint = usingKinect->getWorldCoordinateAt(x, y);
                ofVec3f oldPoint = oldPoints[ix][iy][kinectIndex];
                
                if (bInitCellsOnce)
                {//make sure we give good data on the first iteration!
                    oldPoints[ix][iy][kinectIndex] = focusedPoint;
                    oldPoint = focusedPoint;
                }
                ofVec3f newPoint = oldPoint;

                if (!bFocused)
                {
                    float concentrationError = 1.0 - (mellowReading * (1.0f / MellowThreshold)); //[0, 0.5] --> [1, 0]
                    float maxDist = concentrationError * 400.0f;//100.0f;//2000.0f;//1.0f;//10.0f;
                    
                    newPoint = randomWalk(oldPoint, concentrationError, mellowReading);
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
                float kinectDistance = usingKinect->getDistanceAt(x,y);
                //if((kinectDistance > 0 ) && (kinectDistance < 1400))
                if((kinectDistance > 0 ))
                {
                    ofVec3f dVertex[4] = {
                        ofVec3f(-quadSize, -quadSize, 0),
                        ofVec3f(+quadSize, -quadSize, 0),
                        
                        ofVec3f(-quadSize, +quadSize, 0),
                        ofVec3f(+quadSize, +quadSize, 0)
                        
                    };
                    std::vector<ofVec3f> corners;
                    for (int i=0; i<4; i++)
                    {
                        ofColor originalColor = usingKinect->getColorAt(x, y);
                        originalColor.setBrightness(originalColor.getBrightness() * 2.0);
//                        originalColor.setSaturation(originalColor.getSaturation() * 0.8f);
                        mesh.addColor(originalColor);//(usingKinect->getColorAt(x,y));
                       /* if(kinectIndex==1){
                            mesh.addColor(ofColor(255,0,0));
                        }else{
                            mesh.addColor(ofColor(0,255,0));
                        } */
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

    if(e.getName() == "MELLOW READING"){
        ofxUISlider *slider = e.getSlider();
        mellowReading = slider->getScaledValue();
    }
    if(e.getName() == "KINECT TWO: X"){
        ofxUISlider *slider = e.getSlider();
        Kinect2X = slider->getScaledValue();
    }
    if(e.getName() == "KINECT TWO: Y"){
        ofxUISlider *slider = e.getSlider();
        Kinect2Y = slider->getScaledValue();
    }
    if(e.getName() == "KINECT TWO: Z"){
        ofxUISlider *slider = e.getSlider();
        Kinect2Z = slider->getScaledValue();
    }
    if(e.getName() == "QUAD SIZE"){
        ofxUISlider *slider = e.getSlider();
        quadSize = slider->getScaledValue();
    }
    if(e.getName() == "RECEIVE OSC DATA"){
        ofxUIToggle *toggle = e.getToggle();
        receiveOscData = toggle->getValue();
    }
    if(e.getName() == "CAMERA X"){
        ofxUISlider *slider = e.getSlider();
        cameraX = slider->getScaledValue();
    }
    if(e.getName() == "CAMERA Y"){
        ofxUISlider *slider = e.getSlider();
        cameraY = slider->getScaledValue();
    }
    if(e.getName() == "CAMERA Z"){
        ofxUISlider *slider = e.getSlider();
        cameraZ = slider->getScaledValue();
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
