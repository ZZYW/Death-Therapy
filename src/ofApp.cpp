#include "ofApp.h"

//--------------------------------------------------------------




bool freezePointCloud = false;
bool capturedLastMoment = false;
bool startDecompose = false;

bool startTranscend = false;
bool startRising = false;

float volume = 1;
bool receiveOscData;

float cameraX = 522;
float cameraY = 100;
float cameraZ = -500;

float cameraXpos, cameraZpos;

float cameraRotationX;



vector<Particle> stillPointCloud;

void ofApp::setup()
{
    mourn.loadSound("rain.mp3");
    transcend.loadSound("transcendance.mp3");
    bowl.loadSound("bowlsinging.mp3");

    mourn.play();
    mourn.setLoop(true);
    
	ofBackground(0);
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
    
    gui = new ofxUICanvas();
    gui->setTheme(5);
    gui->setColorBack(ofColor(255));
    gui->addFPS();
    
    gui->addSpacer();
    gui->addLabel("EEG Reading");
    gui->addSlider("SENSOR READING", 0.0, 100.0, &sensorReading);
    gui->addToggle("RECEIVE OSC DATA", true);
    
    gui->addSpacer();
    gui->addLabel("CAMERA ADJUSTMENT");
    gui->addSlider("CAMERA X", -2000.0, 2000.0,&cameraX);
    gui->addSlider("CAMERA Y",-2000.0,2000.0,&cameraY);
    gui->addSlider("CAMERA Z", -2000.0, 2000.0, &cameraZ);
    
//    gui->addSlider("CAMERA ROTATION X", 0, 360, &cameraRotationX);
    
    gui->addSpacer();
    gui->addLabel("KINECT ADJUSTMENT");
    
    gui->addSlider("KINECT POSITION X", -1000.0, 1000.0, 0.0);
    gui->addSlider("KINECT POSITION Y", -1000.0, 1000.0, 0.0);
    gui->addSlider("KINECT POSITION Z", -5000.0, 5000.0, -1000.0);
    
    gui->addSlider("KINECT ROTATE X", 0.0, 360.0, 0.0);
    gui->addSlider("KINECT ROTATE Y", 0.0, 360.0, 0.0);
    gui->addSlider("KINECT ROTATE Z", 0.0, 360.0, 0.0);
    
    
    gui->addSlider("QUAD SIZE",1.0,10.0,2.0);
    
    gui->autoSizeToFitWidgets();
    ofAddListener(gui->newGUIEvent, this, &ofApp::guiEvent);
    gui->loadSettings("settings.xml");
    
}


//--------------------------------------------------------------


float changeRate = 0.50; //increase
float changeRate2 = 0.2;//decrease


float cameraAcce = 100;



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
//            if(m.getAddress() =="/muse/elements/experimental/mellow"){
//                sensorReading = m.getArgAsFloat(0);
//            }
//            if(m.getAddress() =="/sensordata/breath"){
            if(m.getAddress() =="/meditation"){
//                sensorReading = m.getArgAsInt32(0);
                 sensorReading = m.getArgAsFloat(0);
                
            }
        }
    }
    

    
    
    //trigger camera rise
    if(sensorReading>50){
        startRising = true;
    }else{
        startRising = false;
    }

    //set camera position
    if(startRising){
        float cameraYNew = cameraY + cameraAcce;
        if(cameraY<cameraYNew){
            cameraY += 0.01;
        }
    }
    
    cam.setPosition(cameraX, cameraY, cameraZ);

    
    cout<<sensorReading<<endl;
    //sensor reading range (0 - 100)
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
void ofApp::drawScene()
{
    ofPushMatrix();
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    drawPointCloud();
    ofDisableBlendMode();
    ofPopMatrix();
}

//-------------------------------------------------------------

float pointCloudFarDistance = 1000;
float kPosX, kPosY, kPosZ;

int scatterIndicator = 0;

void ofApp::drawPointCloud() {
    int w = 640;
    int h = 480;
    
    if(!freezePointCloud){
        ofMesh mesh;
        mesh.setMode(OF_PRIMITIVE_POINTS);
        int step = 2;
        for(int y = 0; y < h; y += step) {
            for(int x = 0; x < w; x += step) {
                if(kinect.getDistanceAt(x, y) > 0 && kinect.getDistanceAt(x, y) < pointCloudFarDistance) {
                    
                    ofColor originalColor = kinect.getColorAt(x, y);
                    originalColor.setBrightness(originalColor.getBrightness() * 1.50);
                    originalColor.setSaturation(originalColor.getSaturation() * 1.2f);
                    mesh.addColor(originalColor);//(usingKinect->getColorAt(x,y));
                    
                    
                    mesh.addVertex(kinect.getWorldCoordinateAt(x, y));
                }
            }
        }
        glPointSize(quadSize);
        ofPushMatrix();
        // the projected points are 'upside down' and 'backwards'
        ofScale(1, -1, -1);
        ofTranslate(kPosX  , kPosY, kPosZ); // center the points a bit
        ofEnableDepthTest();
        mesh.drawVertices();
        ofDisableDepthTest();
        ofPopMatrix();
    }else{
        
        
        ofMesh stillMesh;
        
        
        if(!capturedLastMoment){
            stillMesh.setMode(OF_PRIMITIVE_POINTS);
            int step = 2;
            for(int y = 0; y < h; y += step) {
                for(int x = 0; x < w; x += step) {
                    if(kinect.getDistanceAt(x, y) > 0 && kinect.getDistanceAt(x, y) < pointCloudFarDistance) {
                        
                        ofColor originalColor = kinect.getColorAt(x, y);
                        originalColor.setBrightness(originalColor.getBrightness() * 1.50);
                        originalColor.setSaturation(originalColor.getSaturation() * 1.2f);
                        stillPointCloud.push_back(Particle(kinect.getWorldCoordinateAt(x, y),originalColor));
                    }
                }
            }
            capturedLastMoment = true;
        }else{
            glPointSize(quadSize);
            ofPushMatrix();
            ofScale(1, -1, -1);
            ofTranslate(kPosX  , kPosY, kPosZ); // center the points a bit
            ofEnableDepthTest();
            
            
            if(ofGetFrameNum() % 100 == 0 && scatterIndicator < 100){
                scatterIndicator++;
            }
            
       
            
            for(int i=0;i<stillPointCloud.size()-1;i++){
                
                
                if(startDecompose){
                    if(stillPointCloud[i].flyThreshold < scatterIndicator){
                        stillPointCloud[i].update();
                        stillPointCloud[i].seek(stillPointCloud[i].location + ofVec3f(ofRandom(-10,10),ofRandom(-10,10),ofRandom(-10,10)));
                    }
                }
                
                
                stillMesh.addColor(stillPointCloud[i].color);
                stillMesh.addVertex(stillPointCloud[i].location);
            }
            
            stillMesh.drawVertices();
            ofDisableDepthTest();
            ofPopMatrix();
        }
    }
    

}
//--------------------------------------------------------------



void ofApp::guiEvent(ofxUIEventArgs &e)
{

    if(e.getName() == "SENSOR READING"){
        ofxUISlider *slider = e.getSlider();
        sensorReading = slider->getScaledValue();
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
    if(e.getName() == "KINECT POSITION X"){
        ofxUISlider *slider = e.getSlider();
        kPosX = slider->getScaledValue();
    }
    if(e.getName() == "KINECT POSITION Y"){
        ofxUISlider *slider = e.getSlider();
        kPosY = slider->getScaledValue();
    }
    if(e.getName() == "KINECT POSITION Z"){
        ofxUISlider *slider = e.getSlider();
        kPosZ = slider->getScaledValue();
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
//		oculusRift.reloadShader();
        freezePointCloud = true;
	}
	
    if(key == 'd'){
        startDecompose = true;
    }
    
	if(key == 'l'){
		oculusRift.lockView = !oculusRift.lockView;
	}
	

	if(key == 'r'){
//		oculusRift.reset();
        startTranscend  = true;
        transcend.play();
        transcend.setLoop(true);
	}
    
	if(key == 'h'){
		ofHideCursor();
	}
	if(key == 'H'){
		ofShowCursor();
	}
    
    if(key == 'g'){
        gui->toggleVisible();
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
