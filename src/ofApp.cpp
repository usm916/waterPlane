#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    fbo.allocate(1280, 800, GL_RGBA);
    
    ofDisableArbTex();
    bmd.loadImage("images/floor.jpg");
    bmdTex = &bmd.getTextureReference();
    bmd2.loadImage("images/sphere.jpg");
    bmdTex2 = &bmd2.getTextureReference();
    ofEnableAlphaBlending();
    
    width2 = 1280 / 2;
    height2 = 800 / 2;
    count = 0;
    
    vertices.assign( NUM_DETAILS * NUM_DETAILS, ofVec3f(0) );
    transformedVertices.assign( NUM_DETAILS * NUM_DETAILS * 2, 0.0f );
    // indices;
    uvt.assign(NUM_DETAILS * NUM_DETAILS * 2, 0.0);
    uvt2.assign(NUM_DETAILS * NUM_DETAILS * 2, 0.0);
    
    // [modification] change surface from x-z plane to x-y plane in order to fit together with normal calculation.
    for(int i=2;i<NUM_DETAILS-2;i++)
    {
        for(int j=2;j<NUM_DETAILS-2;j++)
        {
            vertices[ getIndex(j, i) ] = ofVec3f(
                                                (j - (NUM_DETAILS - 1) * 0.5) / NUM_DETAILS * MESH_SIZE,
                                                (i - (NUM_DETAILS - 1) * 0.5) / NUM_DETAILS * MESH_SIZE, 0);
//            cout << vertices[ getIndex(j, i) ] <<endl;
            if( i!=2 && j!=2 )
            {
                indices.push_back( getIndex(i - 1, j - 1) );
                indices.push_back( getIndex(i, j - 1) );
                indices.push_back( getIndex(i, j) );
                indices.push_back( getIndex(i - 1, j - 1) );
                indices.push_back( getIndex(i, j) );
                indices.push_back( getIndex(i - 1, j) );
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
//        for(int j = 0; j < NUM_DETAILS; j++)
//        {
//            heights[i][j] = 0;
//            velocity[i][j] = 0;
//        }
    }
    ofLogNotice() << "vert " << (int)vertices.size();
    ofLogNotice() << "uvt " << (int)uvt.size();
    ofLogNotice() << "indices " << (int)indices.size();
    
    // [modification] Rendering layers
    vboReflection.setVertexData(&vertices[0], (unsigned int)vertices.size(), GL_DYNAMIC_DRAW);
    vboReflection.setTexCoordData(&uvt[0], (unsigned int)uvt.size(), GL_DYNAMIC_DRAW);
    vboReflection.setIndexData(&indices[0], (unsigned int)indices.size(), GL_STATIC_DRAW);
    
    vboRefraction.setVertexData(&vertices[0], (unsigned int)vertices.size(), GL_DYNAMIC_DRAW);
    vboRefraction.setTexCoordData(&uvt2[0], (unsigned int)uvt2.size(), GL_DYNAMIC_DRAW);
    vboRefraction.setIndexData(&indices[0], (unsigned int)indices.size(), GL_STATIC_DRAW);

    refAlpha = 0.4;
    
    setupGUI();
    
    cam3d.setFov(10);
}

void ofApp::setupGUI(){
    // [modification] controlers
    angle = 20;
    viewedAngleV = 20.0;
    viewedAngleH = 0.0;
    refrac = 1.4;
    reflec = 0.4;
    
    gui = new ofxUISuperCanvas("Water Plane params");
    gui->addSlider("AngleV", 0, 80, &viewedAngleV);//(this, 0, 0, "Angle", function(e:Event):void { viewedAngleV = -e.target.value* 0.017453292519943295;}).setSliderParams();
    gui->addSlider("AngleH", 0, 80, &viewedAngleH);//(this, 0, 0, "Angle", function(e:Event):void { viewedAngleV = -e.target.value* 0.017453292519943295;}).setSliderParams();
    gui->addSlider("Refraction", 0, 20, &refrac);//1, 3, 1.4);
    gui->addSlider("Reflection", 0, 40, &reflec);//(0, 1, 0.4);
    gui->autoSizeToFitWidgets();
    ofAddListener( gui->newGUIEvent, this, &ofApp::guiEvent);

}

void ofApp::guiEvent(ofxUIEventArgs &e)
{
    cam3d.lookAt( ofVec3f(0) );
    cam3d.setPosition(cameraPosition);
}

//--------------------------------------------------------------
void ofApp::update(){
    count++;

    updateWaterSurface();
    setMesh();
    transformVertices();

    cam3d.lookAt( ofVec3f(0) );
    cam3d.setPosition(cameraPosition);

    vboReflection.setVertexData(&vertices[0], (unsigned int)vertices.size(), GL_DYNAMIC_DRAW);
    vboReflection.setTexCoordData(&uvt[0], (unsigned int)uvt.size(), GL_DYNAMIC_DRAW, sizeof(float));

    vboRefraction.setVertexData(&vertices[0], (unsigned int)vertices.size(), GL_DYNAMIC_DRAW);
    vboRefraction.setTexCoordData(&uvt2[0], (unsigned int)uvt2.size(), GL_DYNAMIC_DRAW, sizeof(float));

}

void ofApp::updateWaterSurface()
{
    // ---Water simulation---
    for(int i = 1; i < NUM_DETAILS - 1; i++) {
        for(int j = 1; j < NUM_DETAILS - 1; j++) {
            heights[i][j] += velocity[i][j];
            if (heights[i][j] > 100) heights[i][j] = 100;
            else if (heights[i][j] < -100) heights[i][j] = -100;
        }
    }
    
    for(int i = 1; i < NUM_DETAILS - 1; i++) {
        for(int j = 1; j < NUM_DETAILS - 1; j++) {
            velocity[i][j] = (velocity[i][j] +
                              (heights[i - 1][j] + heights[i][j - 1] + heights[i + 1][j] +
                               heights[i][j + 1] - heights[i][j] * 4) * 0.5) * 0.95;
        }
    }
    
    // why dont I use Matrix3D !?
    //            var viewedAngleH:Number =   (mouseX -232.5) / 465 * 40 * 0.017453292519943295,
    //                viewedAngleV:Number = -((mouseY -232.5) / 465 * 40 + 40) * 0.017453292519943295;
    float sx = sin(viewedAngleV/TWO_PI), sy = sin(viewedAngleH/TWO_PI);
    float cx = cos(viewedAngleV/TWO_PI), cy = cos(viewedAngleH/TWO_PI);
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
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    fbo.begin();
    cam3d.begin();
    ofFill();
    ofClear(0,255);
    
    ofSetColor(255);
    ofNoFill();
    
//    bmdTex->bind();
//    vboRefraction.drawElements(GL_TRIANGLES, (unsigned int)indices.size());
//    bmdTex->unbind();
    
    bmdTex->bind();
    vboReflection.drawElements(GL_TRIANGLES, (unsigned int)indices.size());
    bmdTex->unbind();

        // test of vertices
//    for(int i=0;i<vertices.size();i++)
//    {
//        ofRect(vertices[i],1,1);
//    }
    
//    reflectionGraphics.beginBitmapFill(bmd);
//    reflectionGraphics.drawTriangles(transformedVertices, indices, uvt, TriangleCulling.NEGATIVE);
//    reflectionGraphics.endFill();
    cam3d.end();
    fbo.end();
    
    fbo.draw(0,0,1280,800);
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    float width = ofGetWidth();
    float height = ofGetHeight();
    ripple(( x -width*0.5 )/ width * MESH_SIZE, (y -height*0.5)/ height * MESH_SIZE, 11);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    float width = ofGetWidth();
    float height = ofGetHeight();
    ripple(( x -width*0.5 )/ width * MESH_SIZE, (y -height*0.5)/ height * MESH_SIZE, 36);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
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

void ofApp::ripple(float mx, float my, float intensity){
    float dx, dy, acc;
    int imin, jmin, imax, jmax;
    mx = (mx / MESH_SIZE + 0.5) * NUM_DETAILS;
    my = (my / MESH_SIZE + 0.5) * NUM_DETAILS;
    imin = (mx > 5) ? int(mx - 3) : 2;
    jmin = (my > 5) ? int(my - 3) : 2;
    imax = (mx < NUM_DETAILS-5) ? int(mx + 4) : (NUM_DETAILS - 1);
    jmax = (my < NUM_DETAILS-5) ? int(my + 4) : (NUM_DETAILS - 1);
    for (int i=imin; i<imax; i++) {
        for (int j=jmin; j<jmax; j++) {
            dx = mx - i;
            dy = my - j;
            acc = 3 - sqrt(dx * dx + dy * dy);
            if (acc > 0) velocity[i][j] += acc*intensity;
        }
    }
}

void ofApp::setMesh()
{
    // calclate constants
    float rimo = refractiveIndex - 1;
    float xymax = MESH_SIZE * 0.45;
    float ixymax = 1 / xymax;
    
    for(int i=2; i < NUM_DETAILS - 2; i++)
    {
        for(int j=2; j < NUM_DETAILS - 2; j++)
        {
            int index = i * NUM_DETAILS + j;
            ofVec3f* v = &vertices[index];
            v->z = heights[i][j] * 0.15;
            
            // ---Sphere map---
            float nx, ny, nz;
            nx = (heights[i][j] - heights[i - 1][j]) * 0.15;
            ny = (heights[i][j] - heights[i][j - 1]) * 0.15;
            float len = 1.0 / sqrt(nx * nx + ny * ny + 1);
            nx *= len;
            ny *= len;
            nz = len;
            // ちょっと式を変更して平面でもテクスチャが見えるように
            
            uvt[index * 2] = nx * 0.5 + 0.5 + ((i - NUM_DETAILS * 0.5) * INV_NUM_DETAILS * 0.25);
            uvt[index * 2 + 1] = ny * 0.5 + 0.5 + ((NUM_DETAILS * 0.5 - j) * INV_NUM_DETAILS * 0.25);
            
            // [modification] Refraction map
            // incident vector (you can calculate them in the setup if you want faster)
            float dx = v->x - cameraPosition.x;
            float dy = v->y - cameraPosition.y;
            float dz = v->z - cameraPosition.z;
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
                    uvt2[index * 2] = uvt2[index * 2 + 1] = 0.5;
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
                t = (sign * xymax - v->x) / dx;
                s = t * dy + v->y;
                if (-xymax < s && s < xymax)
                {
                    hitz = t * dz + v->z;
                    if (hitz > boxHeight)
                    {
                        r = (boxHeight-v->z) / dz;
                        uvt2[index * 2]     = (dx * r + v->x) * ixymax * 0.25 + 0.5;
                        uvt2[index * 2 + 1] = (dy * r + v->y) * ixymax * 0.25 + 0.5;
                    } else {
                        r = boxHeight / (hitz + boxHeight);
                        uvt2[index * 2]     = sign       * r * 0.5 + 0.5;
                        uvt2[index * 2 + 1] = s * ixymax * r * 0.5 + 0.5;
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
                t = (sign * xymax - v->y) / dy;
                s = t * dx + v->x;
                hitz = t * dz + v->z;
                if (hitz > boxHeight)
                {
                    r = (boxHeight-v->z) / dz;
                    uvt2[index * 2]     = (dx * r + v->x) * ixymax * 0.25 + 0.5;
                    uvt2[index * 2 + 1] = (dy * r + v->y) * ixymax * 0.25 + 0.5;
                }
                else
                {
                    r = boxHeight / (hitz + boxHeight);
                    uvt2[index * 2]     = s * ixymax * r * 0.5 + 0.5;
                    uvt2[index * 2 + 1] = sign       * r * 0.5 + 0.5;
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
    for (i = 0; i < imax; i++) {
        v = &vertices[i];
        if (v != NULL) {
            iz = focalLength / (v->x * m02 + v->y * m12 + v->z * m22 + cameraDistance);
            transformedVertices[i*2]   = (v->x * m00 + v->y * m10 + v->z * m20) * iz + 232.5;
            transformedVertices[i*2+1] = (v->x * m01 + v->y * m11 + v->z * m21) * iz + 232.5;
        }
    }
}
