#pragma once

#include "ofMain.h"
#include "ofxMaxim.h"
#include "ofxRapidLib.h"
#include "ofxOsc.h"

#include <random>

#define PORT 8000

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
	
    // visual stuff
    ofIcoSpherePrimitive icoSphere;
    float radius;
    
    float noiseSpeedFactor;
    float noiseScaleFactor;
    float noiseDisplacementFactor;
    
    ofShader shader;
    ofEasyCam cam;
    ofVec3f lightPos;
    ofVec3f camPos;
    ofMatrix4x4 modelMat;
    ofMatrix4x4 inverseMat;
    ofMatrix4x4 normalMat;
    
    ofImage noiseTex;
    float imageW;
    float imageH;
    
    // audio stuff
    ofSoundStream soundStream;
    void audioOut(ofSoundBuffer& output) override;
    double finalOut;
    
    maxiOsc modulator;
    maxiOsc carrier;
    maxiOsc phasor;
    
    float index;
    float ratio;
    float carrierFreq;
    
    // machine learning stuff
    void randomParams();
    void trainingExamples(float x, float y, float z);
    bool trainModel();
    void modelOutput(float x, float y, float z);
    
    rapidlib::regression reg;
    std::vector<rapidlib::trainingExample> trainingSet;
    
    bool record;
    bool trained;
    bool train;

    // osc stuff
    ofxOscReceiver osc;
    float xVal;
    float yVal;
    float zVal;
};
