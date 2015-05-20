#pragma once

#include "ofMain.h"

#include "ofxUI.h"

class ofApp : public ofBaseApp{

public:
    const int NUM_DETAILS = 96;
    const float INV_NUM_DETAILS = 1.0 / NUM_DETAILS;
    const float MESH_SIZE = 100;

    ofApp()
    {
        viewedAngleH = 0;
        viewedAngleV = -20 * 0.017453292519943295;
        cameraDistance = MESH_SIZE*10;
        focalLength = MESH_SIZE * 4;
        boxHeight = MESH_SIZE*0.75;
        refractiveIndex = 1.4;
    }
    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    void setupGUI();
    void guiEvent(ofxUIEventArgs &e);
    
    void updateWaterSurface();
    void ripple(float mx, float my, float intensity);
private:
    // Functional
    void setMesh();
    void transformVertices();
    int getIndex(int x, int y){ return y * NUM_DETAILS + x; }
    
    ofFbo fbo;
    ofCamera cam3d;
//    ofEasyCam cam3d;
    
    unsigned int count;
    ofImage bmd, bmd2;
    ofTexture *bmdTex, *bmdTex2;
    vector<ofVec3f> vertices;
    vector<float> transformedVertices;
    vector<unsigned int> indices;
    vector<float> uvt;
    vector<float> uvt2;
    float width2;
    float height2;
    vector<vector<float> > heights;
    vector<vector<float> > velocity;
    
    float viewedAngleH;
    float viewedAngleV;
    float cameraDistance;
    float focalLength;
    float boxHeight;
    float refractiveIndex;
    ofVbo vboReflection;
    ofVbo vboRefraction;
    float refAlpha;
    ofVec3f cameraPosition;
    float m00, m01, m02, m10, m11, m12, m20, m21, m22;
    
    ofxUISuperCanvas* gui;
    float angle, refrac, reflec;
};
