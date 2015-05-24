#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    myWater.setup();
}

//--------------------------------------------------------------
void ofApp::update()
{
    myWater.update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
    myWater.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    myWater.keyPressed(key);
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    myWater.mouseDragged(x, y, button);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    myWater.mousePressed(x, y, button);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    myWater.windowResized(w, h);
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}


