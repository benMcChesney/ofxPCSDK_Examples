#pragma once

#include "pxcupipeline.h"
#include "util_pipeline.h"
#include "gesture_render.h"
#include "ofMain.h"

#define NFINGERS 5

class ofxHand
{
	public :
		ofxHand( ) { } 
		~ofxHand( ) { } 

		void setup( int _handIndex , float _inputWidth , float _inputHeight , bool bMirrorX = true , bool bMirrorY = false ) ; 
		void update( ) ;
		void updateFromPipeline ( PXCUPipeline_Instance pipeline , PXCGesture::GeoNode _mNode ) ;

		PXCGesture::GeoNode::Label geoLabel;
		PXCGesture::GeoNode mNode ; 
		ofPoint pos;
		bool bExists;

		float inputWidth , inputHeight ; 
		bool bMirrorX , bMirrorY ; 
		int handIndex ; 

		float openThreshold ; 
		bool bOpen ; 

		struct finger 
		{
			PXCGesture::GeoNode::Label geoLabel;
			ofPoint pos;
			bool	bExists ; 
		};

		finger	fingers[NFINGERS];

		ofPoint screenPosition ; 

		void drawUserRep( ) ; 
		void drawCursor( ) ; 
};