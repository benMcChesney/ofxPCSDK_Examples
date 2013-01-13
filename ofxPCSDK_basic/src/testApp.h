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

#define NFINGERS 5

struct finger {
	PXCGesture::GeoNode::Label geoLabel;
	ofPoint pos;
	bool	bExists ; 

};

struct hand {
	PXCGesture::GeoNode::Label geoLabel;
	ofPoint pos;
	bool bExists;
};

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

		finger	fingers[NFINGERS];
		hand	hands[2];

        int  gestureId;
        Rect faceRect;
        bool isWait;
		int mlw,mlh;
		

		
};

#endif