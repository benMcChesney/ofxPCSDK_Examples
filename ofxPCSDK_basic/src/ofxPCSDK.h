#pragma once

#include "pxcupipeline.h"
#include "util_pipeline.h"
#include "gesture_render.h"

/*

Simple Wrapper for Intel Perceptual Computing SDK
ben McChensey 2010

*/

class ofxPCSDK
{
	public :
		ofxPCSDK( ) { } 
		~ofxPCSDK( ) { } 
		
		void setup ( ) ;
		void update ( ) ;
		void draw ( ) ; 
};