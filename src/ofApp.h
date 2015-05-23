#pragma once

#include "ofMain.h"

#include "ofxUI.h"

#define TORAD 0.017453292519943295

class ofApp : public ofBaseApp{

public:
    int NUM_DETAILS;
//    int NUM_DETAILSY;
    float INV_NUM_DETAILS;
//    float INV_NUM_DETAILSY;
    float MESH_SIZE;

    ofApp()
    {
        NUM_DETAILS = 256;
        INV_NUM_DETAILS = 1.0 / (float)NUM_DETAILS;
//        NUM_DETAILSY = 120;
//        INV_NUM_DETAILSY = 1.0 / (float)NUM_DETAILSY;
        MESH_SIZE = 300.0;
        viewedAngleH = 0.0;
        viewedAngleV = 0.0;
        refAlpha = 0.6;
        refrac = 1.4;
        reflec = 0.4;
        cameraDistance = MESH_SIZE*3;
        focalLength = MESH_SIZE*3;
        boxHeight = MESH_SIZE*0.25;
        refractiveIndex = 1.4;
        camFov = 10.0;
        bDebugDraw = false;
    }
    void setup();
    void update();
    void draw();
    
    void makeCamPos();

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
    void setVbo();
    void updateVbo();
    void transformVertices();
    void changeWindow();
    int getIndex(int x, int y){ return y * NUM_DETAILS + x; }
    
    ofFbo fbo;
    ofCamera cam3d;
    ofPoint msPos, pMsPos;
    
    bool bDebugDraw;
    
    unsigned int count;
    unsigned int reflecMode, refracMode;
    vector<ofTexture* > vRefractionTex, vReflectionTex;
    vector<ofVec3f> vertices;
    vector<float> transformedVertices;
    vector<unsigned int> indices;
    vector<ofVec2f> uvtReflection;
    vector<ofVec2f> uvtRefraction;
    int uvtSize;
    float width, width2;
    float height, height2, adjustHeight;
    vector<vector<float> > heights;
    vector<vector<float> > velocity;
    
    float viewedAngleH;
    float viewedAngleV;
    float cameraDistance, camFov;
    float focalLength;
    float boxHeight;
    float refractiveIndex;
    ofVbo* vboReflection;
    ofVbo* vboRefraction;
    float refAlpha;
    ofVec3f cameraPosition;
    float m00, m01, m02, m10, m11, m12, m20, m21, m22;
    
    ofxUISuperCanvas* gui;
    float angle, refrac, reflec;
};
