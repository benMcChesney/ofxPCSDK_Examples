/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2012 Intel Corporation. All Rights Reserved.

*******************************************************************************/

#include "testApp.h"

struct {
    PXCGesture::Gesture::Label label;
    int id;
}

gesture_list[] = {
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
	ofSetLogLevel ( OF_LOG_WARNING ) ; 
	ofSetVerticalSync(true);
	ofDisableNormalizedTexCoords();
	glEnable(GL_DEPTH_TEST);
   
	//Initialize our pipeline , let's use all the modules that we can
	pipeline=PXCUPipeline_Init((PXCUPipeline)(PXCU_PIPELINE_GESTURE|PXCU_PIPELINE_FACE_LOCATION|PXCU_PIPELINE_FACE_LANDMARK));
    if (!pipeline) {
        return;
    }

	bMirrorX = true ; 
	bMirrorY = false ;

    int w, h;
	//Allocate the texture for RGB
    if (PXCUPipeline_QueryRGBSize(pipeline,&w, &h)) {
        rgbTexture.allocate(w,h,GL_RGBA);
        colorMap=new unsigned char[w*h*4];
		rgbWidth = w ; 
		rgbHeight = h ;
		
		cout << "color map is : " << w << " x " << h << endl ;
    }
	//Allocate the depth texture
    if (PXCUPipeline_QueryDepthMapSize(pipeline,&w, &h)) {
        depthTexture.allocate(w,h,GL_LUMINANCE);
        depthMap=new short[w*h];
		cout << "depth map is : " << w << " x " << h << endl ;
    }

	//Alocate the label texture
    if (PXCUPipeline_QueryLabelMapSize(pipeline,&w, &h)) {
        labelTexture.allocate(w,h,GL_LUMINANCE);
        labelMap=new unsigned char[w*h];
		cout << "label map is : " << w << " x " << h << endl ;
    }
	
	for( int i = 0; i < 2; i++)
	{
		hands[i].setup( i , 320 , 240 , true , false ) ; 
	}

	camera.setDistance( 350.0f ) ; 
	camera.enableMouseInput( ) ; 
	
	maxXRotation = 200 ; 
	maxYRotation = 200 ; 

	ofSetSmoothLighting(true);
	light = ofLight( ) ; 
	light.setAmbientColor( ofColor(55 , 55 , 55) ) ; 
	light.setDiffuseColor( ofColor(255 , 255 , 255) ) ; 
	light.setDirectional() ; 
	light.setOrientation( ofVec3f( -215 , 35, 8 ) );
	//light.setPosition( -25 , -200 , 400 ) ; 
	ofEnableLighting() ; 
	material.setShininess( 75 );

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
						cout << "gesture ID of : " << gestureId << " had occured ! " << endl ; //gesture_list[i] << endl ;
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
					float _x = data.rectangle.x ; 
					float _y = data.rectangle.y ; 
					if ( bMirrorX ) 
						_x = rgbWidth - data.rectangle.x - ( data.rectangle.w * 0.5 ) ; 
					if ( bMirrorY ) 
						_y = rgbHeight - data.rectangle.y ; 
					faceArea = ofRectangle( _x , _y , data.rectangle.w , data.rectangle.h ) ; 

					ofPoint faceCentroid = ofPoint ( faceArea.x + faceArea.width / 2 , faceArea.y + faceArea.height / 2 ) ; 
					faceCentroid.x = faceCentroid.x / rgbWidth ; 
					faceCentroid.y = faceCentroid.y / rgbHeight ; 
					float curXRotation = ofMap( faceCentroid.x , 0.0f , 1.0f , -maxXRotation , maxXRotation ) ;
					float curYRotation = ofMap( faceCentroid.y , 0.0f , 1.0f , -maxYRotation , maxYRotation ) ;
					ofQuaternion yRot( curXRotation , ofVec3f( 0,-1,0 ) );  
					ofQuaternion xRot( curYRotation , ofVec3f( -1,0,0 ) );  
					curRotation = yRot*xRot;
				}
			}

			//Update hands
			for(int i=0; i <2; i++)
			{
				hands[i].updateFromPipeline( pipeline , mNode ) ; 
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

	camera.begin() ; 
		material.begin() ; 
			ofEnableLighting() ; 
			glEnable( GL_DEPTH_TEST ) ; 
			light.enable( ) ; 
				ofSetColor( 255 , 128 , 128 ) ; 
				ofPushMatrix() ;
					ofVec3f axis;  
					float angle;  
					curRotation.getRotate(angle, axis);  
					//apply the quaternion's rotation to the viewport and draw the sphere
					ofRotate(angle, axis.x, axis.y, axis.z);  
					ofBox( 125 ) ; 
				ofPopMatrix(); 
			light.disable() ;
		material.end( ) ; 
	camera.end( ) ; 

	ofDisableLighting() ; 
	glDisable( GL_DEPTH_TEST ) ; 
	ofSetColor( 255 , 255 , 255 ) ; 

	//Draw the User Representation
	glDisable( GL_DEPTH_TEST ) ; 
	ofRectangle userRepArea = ofRectangle ( ofGetWidth() - 330 , ofGetHeight() - 250 , 320 , 240 ) ; 
	
	//Finger positions align to Depth Map not RGB
	//Flip the texture
	ofPushMatrix() ;
		ofTranslate( userRepArea.x + userRepArea.width , userRepArea.y ) ;
		ofScale( -1 , 1 , 1 ) ; 
		depthTexture.draw( 0 , 0 , userRepArea.width , userRepArea.height ) ; 
	ofPopMatrix() ; 
	//rgbTexture.draw( userRepArea ) ; 

	ofPushMatrix() ; 
		ofTranslate( userRepArea.x , userRepArea.y ) ; 
		for ( int i = 0 ; i < 2 ; i++ ) 
			hands[i].drawUserRep( ) ; 
	ofPopMatrix( ) ;

	ofPushStyle( )  ;
		ofPushMatrix( ) ; 
			ofTranslate( userRepArea.x , userRepArea.y ) ; 
			ofScale( 0.5 , 0.5 ) ; 
			ofRect( userRepArea ) ; 
			ofSetColor( 255 , 255 , 0 ) ; 
			ofNoFill ( ) ; 
			ofEnableSmoothing( ) ; 
			ofSetLineWidth( 2 ) ; 
			ofRect( faceArea ) ; 
		ofPopMatrix( ) ; 
	ofPopStyle( ) ; 

	
	for ( int i = 0 ; i < 2 ; i++ ) 
		hands[i].drawCursor( ) ; 

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
