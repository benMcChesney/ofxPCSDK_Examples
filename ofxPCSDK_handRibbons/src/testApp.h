/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2012 Intel Corporation. All Rights Reserved.

*******************************************************************************/
/*

https://github.com/MadSciLabs/PXCU_demos/ was extremely helpful as well 

*/

#ifndef __TEST_APP_H__
#define __TEST_APP_H__

#include "ofMain.h"
#include "pxcupipeline.h"
#include "util_pipeline.h"
#include "gesture_render.h"
#include "ofxHand.h"
#include "ofxTweenzor.h"
#include "ofEasyCam_DepthCamera.h"
#include "Ribbon.h"
#include "ColorPool.h"
#include "Agent.h"
#include "ofxUI.h"


class testApp : public ofBaseApp {
	public:
	    testApp();
	    virtual ~testApp();
		void setup();
		void update( ) ; 
		void draw();
		void keyPressed(int key);

    protected:
        void release();
    	void updateTextures();
        void convertToImage(unsigned char* _dst, unsigned short* _src, int w, int h, bool invers);
        bool checkImage(unsigned char* _image, int w, int h, unsigned char val);


		void mousePressed( int x , int y , int button ) ; 
		void mouseDragged( int x , int y , int button ) ;
		void mouseReleased( int x , int y , int button ) ;
        //Store all of our textures
		ofTexture rgbTexture ; 
		ofTexture depthTexture ; 
		ofTexture labelTexture ;
		ofRectangle faceArea ;

		//Pipeline is how all the data comes in through the PCSDK
        PXCUPipeline_Instance pipeline;
        unsigned char  *colorMap;
        unsigned char  *labelMap;
        short *depthMap;
        short *irMap;
		
		PXCGesture::GeoNode mNode;
		ofxHand	hand ; 

        int  gestureId;
        Rect faceRect;
        bool isWait;
		int rgbWidth, rgbHeight ;
		bool bMirrorX , bMirrorY ; 
		float faceInterpolateTime ; 
		
		float maxXRotation ; 
		float maxYRotation ; 
		ofQuaternion curRotation ; 
		ofPoint normalizedHeadPosition ; 
		ofLight light ; 
		ofMaterial material ; 

		float curXRotation ; 
		float curYRotation ; 

		//Ribbons stuff !
		enum ApplicationMode  
		{
			CAMERA_MOVEMENT = 0 , 
			DRAW_RIBBON = 1 , 
			ADJUST_COLOR = 2 
		};
		ApplicationMode mode ; 

		//our camera objects for looking at the scene 
		ofEasyCam_DepthCamera camera;
		//_DepthCamera 
		//if usecamera is true, we'll turn on the camera view
		bool usecamera;

		 int numRibbons ; 
    
		vector<Agent*> agents ; 
    
		bool bDrawPath ; 
    
		ColorPool colorPool ; 
    
		ofxUICanvas *gui;   	
		void guiEvent(ofxUIEventArgs &e);
		bool drawPadding; 
    
		void setupUI( ) ; 
    
		float maxForce ; 
		float maxForceRandom ; 
		float maxSpeed ; 
		float maxSpeedRandom ; 
		float bufferDistance ; 
    
		float tailLength ; 
		float tailLengthRandom ;
		float thickness ; 
		float thicknessRandom ; 
    
		void updateAgents() ;
		void updateAgentTrails() ;
    
		float numParticles ; 
    
		void createAgents( bool bFirstMake = false ) ; 

		ofImage cameraIcon ; 
		ofImage drawIcon ; 

		void gesutrePoseEvent ( int gestureId ) ; 
};

#endif