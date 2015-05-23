#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetLogLevel(OF_LOG_SILENT);
    fbo.allocate(1280, 800, GL_RGBA);
    
    ofDisableArbTex();
    
    ofDirectory dirBase;
    ofDirectory dirSky;
    dirBase.listDir("images/base");
    dirSky.listDir("images/sky");
    for(int i=0;i<dirBase.size();i++)
    {
        vRefractionTex.push_back( new ofTexture() );
        ofLoadImage( *vRefractionTex[i], dirBase.getPath(i) );
    }
    for(int i=0;i<dirSky.size();i++)
    {
        vReflectionTex.push_back( new ofTexture() );
        ofLoadImage( *vReflectionTex[i], dirSky.getPath(i) );
    }
    ofEnableAlphaBlending();
    
    reflecMode = 0;
    refracMode = 0;
    
    changeWindow();
    count = 0;
    
    ofLog() << "MESH_SIZE " << MESH_SIZE << endl;
    
    transformedVertices.assign( NUM_DETAILS * NUM_DETAILS * 2, 0.0f );
    // indices;
    uvtSize = NUM_DETAILS * NUM_DETAILS;//(int)sizeof(uvtRefraction)/sizeof(float);
    for(int i=0;i<uvtSize;i++)
    {
        uvtReflection.push_back(ofVec2f(0.0f));
        uvtRefraction.push_back(ofVec2f(0.0f));
    }
    
    // [modification] change surface from x-z plane to x-y plane in order to fit together with normal calculation.
    vertices.assign( NUM_DETAILS * NUM_DETAILS, ofVec3f(0) );
    for(int i=2;i<NUM_DETAILS-2;i++)
    {
        for(int j=2;j<NUM_DETAILS-2;j++)
        {
            vertices[ getIndex(j,i) ] = ofVec3f((j-(NUM_DETAILS-1) * 0.5) / NUM_DETAILS * MESH_SIZE,
                                                 (i-(NUM_DETAILS-1) * 0.5) / NUM_DETAILS * MESH_SIZE, 0);
            if( i!=2 && j!=2 )
            {
                indices.push_back( getIndex(i, j) );
                indices.push_back( getIndex(i, j-1) );
                indices.push_back( getIndex(i-1, j-1) );
                
                indices.push_back( getIndex(i, j) );
                indices.push_back( getIndex(i-1, j-1) );
                indices.push_back( getIndex(i-1, j) );
            }
        }
    }
    
    // initializing vbo mesh
    heights.resize(NUM_DETAILS);
    velocity.resize(NUM_DETAILS);
    for(int i=0;i<NUM_DETAILS;i++)
    {
        heights[i].assign( NUM_DETAILS, 0.0);
        velocity[i].assign( NUM_DETAILS, 0.0);
    }
    ofLogNotice() << "vert    : " << (int)vertices.size();
    ofLogNotice() << "uvt     : " << uvtReflection.size() << " - " << uvtSize;
    ofLogNotice() << "indices : " << (int)indices.size();
    
    // [modification] Rendering layers
    setVbo();
    setupGUI();
    makeCamPos();
}

void ofApp::setVbo()
{
    vboReflection = new ofVbo();
//    ofLog() << "reflec " << &uvtReflection[0] << " : " << uvtReflection;
    vboReflection->setIndexData(&indices[0], (int)indices.size(), GL_STATIC_DRAW);
    
    vboRefraction = new ofVbo();
//    ofLog() << "refrac " << &uvtRefraction[0] << " : " << uvtRefraction;
    vboRefraction->setIndexData(&indices[0], (int)indices.size(), GL_STATIC_DRAW);
    
    updateVbo();
}

void ofApp::setupGUI()
{
    // [modification] controlers
    gui = new ofxUISuperCanvas("Water Plane params");
    gui->addSlider("AngleV", -60, 60, &viewedAngleV);
    gui->addSlider("AngleH", -45, 45, &viewedAngleH);
    gui->addSlider("Fov", 0, 40, &camFov);
    gui->addSlider("Refraction", 0, 20, &refractiveIndex);
    gui->addSlider("Reflection", 0, 1.0, &refAlpha);
    gui->autoSizeToFitWidgets();
    ofAddListener( gui->newGUIEvent, this, &ofApp::guiEvent);
}

void ofApp::guiEvent(ofxUIEventArgs &e)
{
    makeCamPos();
}

//--------------------------------------------------------------
void ofApp::update()
{
    
//    if(ofGetFrameNum()%180==0)ofLog() << ofGetFrameRate();
    pMsPos = msPos;
    msPos = ofPoint(ofGetMouseX(), ofGetMouseY());
    count++;

    updateWaterSurface();
    setMesh();
    transformVertices();

    cam3d.lookAt( ofVec3f(0) );
    cam3d.setPosition(cameraPosition);
    
    updateVbo();
}

void ofApp::updateVbo()
{
    //
    vboReflection->setVertexData( &vertices[0], (int)vertices.size(), GL_DYNAMIC_DRAW);
    vboReflection->setTexCoordData( &uvtReflection[0], uvtSize, GL_DYNAMIC_DRAW);
    //
    vboRefraction->setVertexData( &vertices[0], (int)vertices.size(), GL_DYNAMIC_DRAW);
    vboRefraction->setTexCoordData( &uvtRefraction[0], uvtSize, GL_DYNAMIC_DRAW);
}

void ofApp::makeCamPos()
{
    float sx = sin(viewedAngleV*TORAD), sy = sin(viewedAngleH*TORAD);
    float cx = cos(viewedAngleV*TORAD), cy = cos(viewedAngleH*TORAD);
    m00 = cy;
    m01 = 0;
    m02 = -sy;
    m10 = sy*sx;
    m11 = cx;
    m12 = cy*sx;
    m20 = sy*cx;
    m21 = -sx;
    m22 = cy*cx;
    cameraPosition.x = -cameraDistance * m02;
    cameraPosition.y = -cameraDistance * m12;
    cameraPosition.z = -cameraDistance * m22;
    
    cam3d.lookAt( ofVec3f(0) );
    cam3d.setPosition( cameraPosition );
    cam3d.setFov(camFov);
}

//--------------------------------------------------------------
void ofApp::draw()
{
    
//    ofEnableBlendMode(OF_BLENDMODE_ADD);
    fbo.begin();
    cam3d.begin();
    ofFill();
    ofClear(0,255);
    
    ofSetColor(255);
    ofNoFill();
    
    vRefractionTex[refracMode]->bind();
    vboRefraction->drawElements(GL_TRIANGLES, (unsigned int)indices.size());
    vRefractionTex[refracMode]->unbind();
    
    ofSetColor(255,255*refAlpha);
    vReflectionTex[reflecMode]->bind();
    vboReflection->drawElements(GL_TRIANGLES, (unsigned int)indices.size());
    vReflectionTex[reflecMode]->unbind();

    // test of vertices
    if(bDebugDraw)
    {
        ofLine(-MESH_SIZE,0,MESH_SIZE,0);
        ofLine(0,-MESH_SIZE,0,MESH_SIZE);
        ofSetColor(255,128,0);
        ofLog() << "vert " << vertices.size();
        
        for(int i=0;i<vertices.size();i++)
        {
            ofRect( vertices[i].x, vertices[i].y, vertices[i].z, 1, 1 );
        }
    }
    cam3d.end();
    fbo.end();
    
    // DRAW FINAL RESULT
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    ofSetColor(255);
    fbo.draw(0,0,1280,800);
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    switch (key)
    {
        case '[':
            refracMode = (refracMode+1)%(vRefractionTex.size());
            break;
        case ']':
            refracMode = (refracMode-1)%(vRefractionTex.size());
            break;
        case '-':
            reflecMode = (reflecMode+1)%(vReflectionTex.size());
            break;
        case '=':
            reflecMode = (reflecMode-1)%(vReflectionTex.size());
            break;
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    float adjustWidth = 50;
    float sWidth = width + 100;
    ripple( ( width*0.5-(adjustHeight + y) )/ width * MESH_SIZE, (sWidth*0.5-(adjustWidth+x) )/ sWidth * MESH_SIZE, 5);
    ofPoint half = msPos+(pMsPos-msPos)*0.4;
    ripple( ( width*0.5-(adjustHeight + half.y) )/ width * MESH_SIZE, (sWidth*0.5-(adjustWidth+half.x) )/ sWidth * MESH_SIZE, 3);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    float adjustWidth = 50;
    float sWidth = width + 100;
    ripple( ( width*0.5-(adjustHeight + y) )/ width * MESH_SIZE, (sWidth*0.5-(adjustWidth+x) )/ sWidth * MESH_SIZE, 9);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    changeWindow();
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

void ofApp::ripple(float mx, float my, float intensity){
    float dx, dy, acc;
    int imin, jmin, imax, jmax;
    mx = (mx / MESH_SIZE + 0.5) * NUM_DETAILS;
    my = (my / MESH_SIZE + 0.5) * NUM_DETAILS;
    imin = (mx > 5) ? int(mx-3) : 2;
    jmin = (my > 5) ? int(my-3) : 2;
    imax = (mx < NUM_DETAILS-5) ? int(mx + 4) : (NUM_DETAILS-1);
    jmax = (my < NUM_DETAILS-5) ? int(my + 4) : (NUM_DETAILS-1);
    for (int i=imin; i<imax; i++)
    {
        for (int j=jmin; j<jmax; j++)
        {
            dx = mx-i;
            dy = my-j;
            acc = 3-sqrt(dx * dx + dy * dy);
            if (acc > 0) velocity[i][j] += acc*intensity;
        }
    }
}

void ofApp::updateWaterSurface()
{
    // ---Water simulation---
    for(int i = 1; i < NUM_DETAILS-1; i++) {
        for(int j = 1; j < NUM_DETAILS-1; j++) {
            heights[i][j] += velocity[i][j];
            if (heights[i][j] > 100) heights[i][j] = 100;
            else if (heights[i][j] < -100) heights[i][j] = -100;
        }
    }
    
    for(int i = 1; i < NUM_DETAILS-1; i++) {
        for(int j = 1; j < NUM_DETAILS-1; j++) {
            velocity[i][j] = (velocity[i][j] +
                              (heights[i-1][j] + heights[i][j-1] + heights[i + 1][j] +
                               heights[i][j + 1]-heights[i][j] * 4) * 0.5) * 0.95;
        }
    }
    
    // why dont I use Matrix3D !?
    //            var viewedAngleH:Number =   (mouseX -232.5) / 465 * 40 * 0.017453292519943295,
    //                viewedAngleV:Number = -((mouseY -232.5) / 465 * 40 + 40) * 0.017453292519943295;
    makeCamPos();
}

void ofApp::setMesh()
{
    // calclate constants
    float rimo = refractiveIndex - 1.0;
    float xymax = 0.45 * MESH_SIZE;
    float ixymax = 1.0 / xymax;
    
    for(int i=2; i < NUM_DETAILS-2; i++)
    {
        for(int j=2; j < NUM_DETAILS-2; j++)
        {
            int index = i * NUM_DETAILS + j;
            ofVec3f* v = &vertices[index];
            v->z = heights[i][j] * 0.15;
            
            // ---Sphere map---
            float nx, ny, nz;
            nx = (heights[i][j]-heights[i-1][j]) * 0.15;
            ny = (heights[i][j]-heights[i][j-1]) * 0.15;
            float len = 1.0 / sqrt(nx * nx + ny * ny + 1);
            nx *= len;
            ny *= len;
            nz = len;
            uvtReflection[index].x = nx * 0.5 + 0.5 + ((i-NUM_DETAILS * 0.5) * INV_NUM_DETAILS * 0.25);
            uvtReflection[index].y = ny * 0.5 + 0.5 + ((NUM_DETAILS * 0.5-j) * INV_NUM_DETAILS * 0.25);
            
            // [modification] Refraction map
            // incident vector (you can calculate them in the setup if you want faster)
            float dx = v->x-cameraPosition.x;
            float dy = v->y-cameraPosition.y;
            float dz = v->z-cameraPosition.z;
            len = 1 / sqrt(dx * dx + dy * dy + dz * dz);
            dx *= len;
            dy *= len;
            dz *= len;
            // output vector
            float t, s, r, hitz, sign;
            t = (dx * nx + dy * ny + dz) * rimo;
            dx += nx * t;
            dy += ny * t;
            dz += nz * t;
            // in this calculation, we can omit normalization of output vector !
            //len = 1 / Math.sqrt(dx * dx + dy * dy + dz * dz);
            //dx *= len;
            //dy *= len;
            //dz *= len;
            
            // uv coordinate
            if (dx == 0)
            {
                if (dy == 0)
                {
                    uvtRefraction[index].x = uvtRefraction[index].y = 0.5;
                    sign = 0;
                }
                else
                {
                    sign = (dy < 0) ? -1 : 1;
                }
            }
            else
            {
                sign = (dx < 0) ? -1 : 1;
                t = (sign * xymax-v->x) / dx;
                s = t * dy + v->y;
                if (-xymax < s && s < xymax)
                {
                    hitz = t * dz + v->z;
                    if (hitz > boxHeight)
                    {
                        r = (boxHeight-v->z) / dz;
                        uvtRefraction[index].x     = (dx * r + v->x) * ixymax * 0.25 + 0.5;
                        uvtRefraction[index].y = (dy * r + v->y) * ixymax * 0.25 + 0.5;
                    } else {
                        r = boxHeight / (hitz + boxHeight);
                        uvtRefraction[index].x     = sign       * r * 0.5 + 0.5;
                        uvtRefraction[index].y = s * ixymax * r * 0.5 + 0.5;
                    }
                    sign = 0;
                }
                else
                {
                    sign = (s < 0) ? -1 : 1;
                }
            }
            
            if(sign != 0)
            {
                t = (sign * xymax-v->y) / dy;
                s = t * dx + v->x;
                hitz = t * dz + v->z;
                if (hitz > boxHeight)
                {
                    r = (boxHeight-v->z) / dz;
                    uvtRefraction[index].x     = (dx * r + v->x) * ixymax * 0.25 + 0.5;
                    uvtRefraction[index].y = (dy * r + v->y) * ixymax * 0.25 + 0.5;
                }
                else
                {
                    r = boxHeight / (hitz + boxHeight);
                    uvtRefraction[index].x     = s * ixymax * r * 0.5 + 0.5;
                    uvtRefraction[index].y = sign       * r * 0.5 + 0.5;
                }
            }
        }
    }
}

void ofApp::transformVertices()
{
    float iz;
    ofVec3f* v;
    int i;
    int imax = vertices.size();
    for (i = 0; i < imax; i++)
    {
        v = &vertices[i];
        if (v != NULL) {
            iz = focalLength / (v->x * m02 + v->y * m12 + v->z * m22 + cameraDistance);
            transformedVertices[i*2]   = (v->x * m00 + v->y * m10 + v->z * m20) * iz + 232.5;
            transformedVertices[i*2+1] = (v->x * m01 + v->y * m11 + v->z * m21) * iz + 232.5;
        }
    }
}

void ofApp::changeWindow()
{
    width = ofGetWidth();
    height = ofGetHeight();
    width2 = width/2;
    height2 = height/2;
    adjustHeight = (width-height)/2;
}


