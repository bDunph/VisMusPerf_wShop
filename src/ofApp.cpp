#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    // osc stuff
    osc.setup(PORT);
    
    // machine learning stuff
    record = false;
    trained = false;
    train = false;
    
    // audio stuff
    int sampleRate = 44100;
    int bufferSize = 512;
    ofxMaxiSettings::setup(sampleRate, 2, bufferSize);
    
    ofSoundStreamSettings settings;
    settings.setOutListener(this);
    settings.sampleRate = sampleRate;
    settings.numOutputChannels = 2;
    settings.numInputChannels = 0;
    settings.bufferSize = bufferSize;
    soundStream.setup(settings);
    
    index = 15.0f;
    ratio = 2.1f;
    carrierFreq = 200.0f;
    
    // visual stuff
    ofEnableDepthTest();
    ofSetSmoothLighting(true);
    
    radius = 100.0f;
    
    lightPos = ofVec3f(502.0f, 502.0f, -501.0f);
    
    noiseSpeedFactor = 6.0f;
    noiseScaleFactor = 0.5f;
    noiseDisplacementFactor = 0.0f;

    imageW = 80.0f;
    imageH = 60.0f;
    noiseTex.allocate(imageW, imageH, OF_IMAGE_GRAYSCALE);
    
    icoSphere.mapTexCoordsFromTexture(noiseTex.getTexture());
    shader.load("shader");
    
}

//--------------------------------------------------------------
void ofApp::update(){

    // osc stuff
    while(osc.hasWaitingMessages()){
        
        // get the next message
        ofxOscMessage m;
        osc.getNextMessage(m);
        
        if(m.getAddress() == "/midi/cc6/1"){
            xVal = m.getArgAsFloat(0);
        } else if(m.getAddress() == "midi/cc7/1"){
            yVal = m.getArgAsFloat(0);
        } else if(m.getAddress() == "midi/cc8/1"){
            zVal = m.getArgAsFloat(0);
        }
    }
    
    // machine learning stuff
    
    // for mouse controls
    if(record) trainingExamples(ofGetMouseX(), ofGetMouseY(), 0);
    
    // for osc controls
//    if(record) trainingExamples(xVal, yVal, zVal);
    
    if(train) trained = trainModel();
    
    // for osc controls
//    if(trained) modelOutput(xVal, yVal, zVal);
    
    // visual stuff
    camPos = cam.getGlobalPosition();
    
    icoSphere.setRadius(radius);
    
    modelMat = icoSphere.getLocalTransformMatrix();
    inverseMat = inverseMat.getInverse();
    normalMat = inverseMat.getTransposedOf(inverseMat);
    
    float noiseScale = noiseScaleFactor;
    float noiseVel = ofGetElapsedTimef() * noiseSpeedFactor;

    ofPixels & pixels = noiseTex.getPixels();
    int w = noiseTex.getWidth();
    int h = noiseTex.getHeight();
    for(int y=0; y<h; y++) {
        for(int x=0; x<w; x++) {
            int i = y * w + x;
            float noiseVelue = ofNoise(x * noiseScale, y * noiseScale, noiseVel);
            pixels[i] = 255 * noiseVelue;
        }
    }
    noiseTex.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofEnableLighting();
    ofSetBackgroundColor(0.0f, 0.0f, 0.0f);
    
    noiseTex.getTexture().bind();
    shader.begin();

    ofPushMatrix();
    ofTranslate(ofGetWidth() * 0.5f, ofGetHeight() * 0.5f);
    cam.begin();

    shader.setUniform1f("noiseDisplacementFactor", noiseDisplacementFactor);
    shader.setUniform3f("lightPos", lightPos);
    shader.setUniform3f("camPos", camPos);
    shader.setUniformMatrix4f("normalMat", normalMat);

    icoSphere.draw();

    cam.end();
    ofPopMatrix();

    shader.end();
    noiseTex.getTexture().unbind();
    
    // onscreen feedback
    stringstream s;
    
    s << "trained: " << trained;
    ofDrawBitmapString(s.str(), ofGetWindowWidth()*0.1, 10);
    s.str("");
    s << "recording: " << record;
    ofDrawBitmapString(s.str(), ofGetWindowWidth()*0.3, 10);
    s.str("");
}

//--------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer& output){
        
    std::size_t outChannels = output.getNumChannels();
    
    for(int i = 0; i < output.getNumFrames(); ++i){
        
        float modFreq = carrierFreq / ratio;
        float deviation = index * modFreq;
        
        double modSignal = modulator.saw(modFreq) * deviation;
        double carrierSignal = carrier.saw(carrierFreq + modSignal) * 0.3f;
        
        finalOut = carrierSignal;
        
        if(finalOut > 0.95f){
            finalOut = 0.95f;
        }
        
        output[i * outChannels] = finalOut;
        output[i * outChannels + 1] = output[i * outChannels];
    }
}

//--------------------------------------------------------------
void ofApp::randomParams(){
    
    std::random_device rd;
    std::default_random_engine generator(rd());
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    
    // visual parameters
    radius = 500.0f * distribution(generator);
    if(radius < 10.0f) radius = 10.0f;
    if(radius > 500.0f) radius = 500.0f;
    
    noiseScaleFactor = 30.0f * distribution(generator);
    if(noiseScaleFactor < 0.0f) noiseScaleFactor = 0.0f;
    if(noiseScaleFactor > 30.0f) noiseScaleFactor = 30.0f;
    
    noiseSpeedFactor = 20.0f * distribution(generator);
    if(noiseSpeedFactor < 0.0f) noiseSpeedFactor = 0.0f;
    if(noiseSpeedFactor > 20.0f) noiseSpeedFactor = 20.0f;

    noiseDisplacementFactor = 100.0f * distribution(generator);
    if(noiseDisplacementFactor < 0.0f) noiseDisplacementFactor = 0.0f;
    if(noiseDisplacementFactor > 100.0f) noiseDisplacementFactor = 100.0f;
    
    // audio parameters
    index = 50.0f * distribution(generator);
    if(index < 0.0f) index = 0.0f;
    if(index > 50.0f) index = 50.0f;
    
    ratio = 10.0f * distribution(generator);
    if(ratio < 0.0f) ratio = 0.1f;
    if(ratio > 10.0f) ratio = 10.0f;
    
    carrierFreq = 800.0f * distribution(generator);
    if(carrierFreq < 20.0f) carrierFreq = 20.0f;
    if(carrierFreq > 800.0f) carrierFreq = 800.0f;
}

//--------------------------------------------------------------
void ofApp::trainingExamples(float x, float y, float z){
    
    rapidlib::trainingExample tempExample;
    
    tempExample.input = {(double) x, (double) y, (double) z};
    
    tempExample.output = {(double) radius, (double) noiseScaleFactor, (double) noiseSpeedFactor, (double) noiseDisplacementFactor, (double) index, (double) ratio, (double) carrierFreq};

    trainingSet.push_back(tempExample);
}

//--------------------------------------------------------------
bool ofApp::trainModel(){
    bool result = reg.train(trainingSet);
    return result;
}

void ofApp::modelOutput(float x, float y, float z){
    std::vector<double> input;
    std::vector<double> output;
    
    input.push_back ((double)x);
    input.push_back ((double)y);
    input.push_back ((double)z);
    
    output = reg.run(input);
    
    radius = output[0];
    if(output[0] < 10.0f){
        radius = 10.0f;
    }
    
    noiseScaleFactor = output[1];
    if(output[1] < 0.0f){
        noiseScaleFactor = 0.0f;
    } else if (output[1] > 30.0f){
        noiseScaleFactor = 30.0f;
    }
    
    noiseSpeedFactor = output[2];
    if(output[2] < 0.0f){
        noiseSpeedFactor = 0.0f;
    } else if(output[2] > 20.0f){
        noiseSpeedFactor = 20.0f;
    }
    
    noiseDisplacementFactor = output[3];
    if(output[3] < 0.0f){
        noiseDisplacementFactor = 0.0f;
    } else if(output[3] > 100.0f){
        noiseDisplacementFactor = 100.0f;
    }
    
    index = output[4];
    if(output[4] < 0.0f){
        index = 0.0f;
    } else if(output[4] > 50.0f){
        index = 50.0f;
    }
    
    ratio = output[5];
    if(output[5] < 0.0f){
        ratio = 0.0f;
    } else if (output[5] > 10.0f){
        ratio = 10.0f;
    }
    
    carrierFreq = output[6];
    if(output[6] < 20.0f){
        carrierFreq = 20.0f;
    } else if(output[6] > 800.0f){
        carrierFreq = 800.0f;
    }
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    // space bar
    if(key == 32) randomParams();
    
    // r
    if(key == 114) record = true;
    
    // t
    if(key == 116) train = true;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

    // r
    if(key == 114) record = false;
    
    // t
    if(key == 116) train = false;
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

    if(trained) modelOutput(x, y, 0);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
