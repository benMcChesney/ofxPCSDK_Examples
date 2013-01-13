/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2012 Intel Corporation. All Rights Reserved.

*******************************************************************************/

#include "testApp.h"

//Enumerators and Labels
PXCGesture::GeoNode::Label gFingers[] = {	PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY|PXCGesture::GeoNode::LABEL_FINGER_THUMB,
											PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY|PXCGesture::GeoNode::LABEL_FINGER_INDEX,
											PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY|PXCGesture::GeoNode::LABEL_FINGER_MIDDLE,
											PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY|PXCGesture::GeoNode::LABEL_FINGER_RING,
											PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY|PXCGesture::GeoNode::LABEL_FINGER_PINKY
};


PXCGesture::GeoNode::Label gHands[] = {PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY,PXCGesture::GeoNode::LABEL_BODY_HAND_SECONDARY};

ofPoint screenSize;
float screenScale;

struct {
    PXCGesture::Gesture::Label label;
    int id;
} gesture_list[] = {
    PXCGesture::Gesture::LABEL_POSE_PEACE,      1,
    PXCGesture::Gesture::LABEL_POSE_THUMB_UP,   2,
    PXCGesture::Gesture::LABEL_POSE_THUMB_DOWN, 3,

    PXCGesture::Gesture::LABEL_HAND_WAVE,       4,
    PXCGesture::Gesture::LABEL_NAV_SWIPE_LEFT,  5,
    PXCGesture::Gesture::LABEL_NAV_SWIPE_RIGHT, 6,
};

testApp::testApp() 
{
    colorMap=NULL;
    depthMap=NULL;
    irMap=NULL;
    labelMap=NULL;
    gestureId=-1;
    isWait=false;
	pipeline=NULL;
}


void testApp::setup() 
{
	ofSetVerticalSync(true);
	ofDisableNormalizedTexCoords();
	glEnable(GL_DEPTH_TEST);
   
	//Initialize our pipeline , let's use all the modules that we can
	pipeline=PXCUPipeline_Init((PXCUPipeline)(PXCU_PIPELINE_GESTURE|PXCU_PIPELINE_FACE_LOCATION|PXCU_PIPELINE_FACE_LANDMARK));
    if (!pipeline) {
        return;
    }

    int w, h;
	//Allocate the texture for RGB
    if (PXCUPipeline_QueryRGBSize(pipeline,&w, &h)) {
        rgbTexture.allocate(w,h,GL_RGBA);
        colorMap=new unsigned char[w*h*4];
		cout << "color map is : " << w << " , " << h << endl ;
    }
	//Allocate the depth texture
    if (PXCUPipeline_QueryDepthMapSize(pipeline,&w, &h)) {
        depthTexture.allocate(w,h,GL_LUMINANCE);
        depthMap=new short[w*h];
		cout << "depth map is : " << w << " , " << h << endl ;
    }

	//Alocate the label texture
    if (PXCUPipeline_QueryLabelMapSize(pipeline,&w, &h)) {
        labelTexture.allocate(w,h,GL_LUMINANCE);
        labelMap=new unsigned char[w*h];
		mlw = w ; 
		mlh = h ; 
		cout << "label map is : " << w << " , " << h << endl ;
    }
	
	//Setup the fingers
	for(int i=0;i<NFINGERS;++i)
	{	
		fingers[i].bExists = false;
		fingers[i].geoLabel = gFingers[i];
		fingers[i].pos.set(0,0,0);
	}

	for(int i=0; i< 2; i++){
		//setup hands stuff
		hands[i].bExists = false;
		hands[i].geoLabel = gHands[i];
		hands[i].pos.set(0,0,0);
	}
	
	screenScale = 4.0; // some scalar for screen positioning // not accurate
}

void testApp::update( ) 
{
	updateTextures() ;
}

void testApp::convertToImage(unsigned char* _dst, unsigned short* _src, int w, int h, bool invers) {
    float minC = 0xffff;
    float maxC = -0xffff;
    for(int k=0;k<w*h;k++) {
        float vC = (float)_src[k]/0xffff;
        if (minC>vC) minC = vC;
        if (maxC<vC) maxC = vC;
    }
    for(int i=0;i<w*h;i++) {
        float val = (float)_src[i]/0xffff;
        val = 255.f*sqrt((val-minC)/(maxC-minC));
        _dst[i]=invers?255-(unsigned char)val:(unsigned char)val;
    }
}

bool testApp::checkImage(unsigned char* _image, int w, int h, unsigned char val) {
    unsigned char* image = _image;
    for (int i=0;i<w*h;i++) {
        if (image[0]<val) return true;
        image += 4;
    }
    return false;
}

void testApp::updateTextures() 
{
	//Make sure the pipeline is valid
   if (pipeline) 
   {
	   //Lock in the frame and use it's data
	   if (PXCUPipeline_AcquireFrame(pipeline,false)) 
	   {
		   //Query each texture data
			if (colorMap) PXCUPipeline_QueryRGB(pipeline,colorMap);
			if (depthMap) PXCUPipeline_QueryDepthMap(pipeline,depthMap);
			if (irMap)    PXCUPipeline_QueryIRMap(pipeline,irMap);
			if (labelMap) PXCUPipeline_QueryLabelMap(pipeline,labelMap,0);

			//Query the gesture data
			PXCGesture::Gesture data;
			if (PXCUPipeline_QueryGesture(pipeline,PXCGesture::GeoNode::LABEL_ANY,&data))
			{
				gestureId=-1;
				//Check which gesture has been activated
				for (int i=0;i<sizeof(gesture_list)/sizeof(gesture_list[0]);i++)
				{
					if (gesture_list[i].label == data.label) 
					{
						gestureId=gesture_list[i].id;
						cout << "gesture ID of : " << gestureId << " had occured ! " << endl ;
						break;
					}
				}
			}

			//Get face tracking data
			pxcUID face;
			pxcU64 timeStamp;
			faceRect.left = -1;
			if (PXCUPipeline_QueryFaceID(pipeline,0, &face, &timeStamp)) 
			{
				PXCFaceAnalysis::Detection::Data data;
				if (PXCUPipeline_QueryFaceLocationData(pipeline, face, &data)) 
				{

					faceArea = ofRectangle( data.rectangle.x , data.rectangle.y , data.rectangle.w , data.rectangle.h ) ; 
				}
			}

			//Update all the fingers
			int nFingersDetected = 0;
			for(int i=0;i<NFINGERS;++i)
			{
				fingers[i].bExists = PXCUPipeline_QueryGeoNode(pipeline, fingers[i].geoLabel, &mNode);
				if(fingers[i].bExists ) 
				{	
					nFingersDetected ++;
					fingers[i].pos.set(mlw - mNode.positionImage.x, mNode.positionImage.y, mNode.positionWorld.z);
				} 
			}
			
			//Update hands
			for(int i=0; i <2; i++)
			{
				hands[i].bExists = PXCUPipeline_QueryGeoNode( pipeline , hands[i].geoLabel, &mNode);
				if(hands[i].bExists)
				{
					hands[i].pos.set(mlw- mNode.positionImage.x, mNode.positionImage.y, mNode.openness*.01);
				}
			}

			//Now that we are done grabbing data from the frame we can unlock or release it
			PXCUPipeline_ReleaseFrame(pipeline);
			isWait=true;

			//Load all the data into our textures
			if (colorMap) 
			{
				rgbTexture.loadData(colorMap, rgbTexture.getWidth(), rgbTexture.getHeight(), GL_RGBA);
			}
			if (labelMap) 
			{
				labelTexture.loadData(labelMap, labelTexture.getWidth(), labelTexture.getHeight(), GL_LUMINANCE);
				if (depthMap) 
				{
					convertToImage(labelMap, (unsigned short*)depthMap, depthTexture.getWidth(), depthTexture.getHeight(), true);
					depthTexture.loadData(labelMap, depthTexture.getWidth(),depthTexture.getHeight(), GL_LUMINANCE);
				}
			}
		}
   }
}

void testApp::draw() 
{
	ofBackground(15 , 15 , 15 );
	ofEnableAlphaBlending( ); 

	ofSetColor(255);

	ofSetColor( 255 , 255 , 255 ) ; 

	//Draw the User Representation
	glDisable( GL_DEPTH_TEST ) ; 
	ofRectangle userRepArea = ofRectangle ( ofGetWidth() - 330 , ofGetHeight() - 250 , 320 , 240 ) ; 
	
	//depthTexture.draw( userRepArea ) ; 
	rgbTexture.draw( userRepArea ) ; 

	ofPushStyle( )  ;
		ofPushMatrix( ) ; 
			ofTranslate( userRepArea.x , userRepArea.y ) ; 
			ofRect( userRepArea.x / 2 , userRepArea.y / 2 , userRepArea.width/2 , userRepArea.height/2 ) ; 
			ofSetColor( 255 , 255 ,0 ) ; 
			ofNoFill ( ) ; 
			ofEnableSmoothing( ) ; 
			ofSetLineWidth( 2 ) ; 
			ofRect( faceArea ) ; 
		ofPopMatrix( ) ; 
	ofPopStyle( ) ; 

	ofPushMatrix( ) ; 
		ofTranslate( userRepArea.x , userRepArea.y ) ;
		ofPushStyle( ) ; 
			for(int i=0;i<NFINGERS;++i)
			{
				fingers[i].bExists = PXCUPipeline_QueryGeoNode(pipeline, fingers[i].geoLabel, &mNode);
				if(fingers[i].bExists ) 
				{	
					ofFill( ) ; 
					ofSetColor( 255 , 0 , 255 ) ;
					ofCircle( mNode.positionImage.x , mNode.positionImage.y , 15 ) ; 
				} 
			}
		ofPopStyle( ) ; 
	ofPopMatrix() ; 
	ofSetColor( 255 , 128 ) ; 
	//rgbTexture.draw( userRepArea ) ; 
	
	for(int i=0;i<NFINGERS;i++)
	{
		if(fingers[i].bExists)
		{
	
			ofSetColor(0,255,255);

			//ofSetColor(255,255,0);
			ofFill();
			ofCircle((ofGetScreenWidth()-mlw) +fingers[i].pos.x, fingers[i].pos.y, 5);
			//ofNoFill();
			ofCircle(fingers[i].pos*screenScale, 20);
		}
	}

	for(int i = 0; i<2; i++){
		if(hands[i].bExists){
			ofSetColor(255);
			ofFill();
			if(i==0)	ofNoFill();
			ofCircle((hands[i].pos*screenScale)+ hands[i].pos.z*.25, (hands[i].pos.z*100)*.5);
		}
	}

	string msg = "\nPress 'f' to toggle fullscreen";
	msg += "\nfps: " + ofToString(ofGetFrameRate(), 2);

	ofDrawBitmapStringHighlight(msg, 10, 20);


}

void testApp::keyPressed(int key){
	switch(key) {
		case 'F':
		case 'f':
			ofToggleFullscreen();
			break;
	}
}


testApp::~testApp() {
    release();
}

void testApp::release() {
    if (isWait) PXCUPipeline_AcquireFrame(pipeline,true); // wait the last frame

    if (colorMap) {delete[] colorMap; colorMap=NULL;}    
    if (depthMap) {delete[] depthMap; depthMap=NULL;}    
    if (irMap) {delete[] irMap; irMap=NULL;}    
    if (labelMap) {delete[] labelMap; labelMap=NULL;}   
}
