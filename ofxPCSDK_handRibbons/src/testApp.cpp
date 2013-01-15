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
	Tweenzor::init( ) ; 

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
	curXRotation = 0; 
	curYRotation = 0 ; 

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
	

	hand.setup( 0 , 320 , 240 , true , false ) ; 

	//camera.setDistance( 350.0f ) ; 
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


	//Initialize Ribbons
	//initialize the variable so it's off at the beginning
    bDrawPath = false ;

    //Add some nice looking colors, acquired from adobe kuler
    ofColor col1 = ofColor ( 0 , 166 , 149 ) ;
    ofColor col2 = ofColor ( 255 , 51 , 0 ) ;
    ofColor col3 = ofColor ( 255 , 102 , 0 ) ;
    ofColor col4 = ofColor ( 255 , 166 , 0 ) ;
    ofColor col5 = ofColor ( 255 , 217 , 0 ) ;

    colorPool.addColor( col1 ) ;
    colorPool.addColor( col2 ) ;
    colorPool.addColor( col3 ) ;
    colorPool.addColor( col4 ) ;
    colorPool.addColor( col5 ) ;

    camera.setPosition( ofGetWidth()/2 , ofGetHeight()/2 , -100 ) ; //ofGetWidth()/2, ofGetHeight()/2, 0 ) ;

    numRibbons = 0 ;
    numParticles = 3 ;
    createAgents( true ) ;
    setupUI() ;

	mode = CAMERA_MOVEMENT ; 
	cameraIcon.loadImage( "camera+4finger.png" ) ; 
	drawIcon.loadImage( "draw+v.png" ) ; 
}

void testApp::update( ) 
{
	bool lastHandOpen = hand.bOpen ; 
	Tweenzor::update( ofGetElapsedTimeMillis() ) ; 
	updateTextures() ;

	hand.update( ) ; 
	
	ofMouseEventArgs args ; 
	args.x = hand.screenPosition.x ; //palmPosition.x ; 
	args.y = hand.screenPosition.y ; //.palmPosition.y ; 
	args.button = 0 ; 

	if ( hand.bExists == true ) 
	{
		if ( hand.bOpen != lastHandOpen )
		{
			//was closed now open 
			if ( hand.bOpen == true ) 
			{
				mouseReleased( hand.screenPosition.x , hand.screenPosition.y , 0 ) ; 
				if ( mode == CAMERA_MOVEMENT )
					camera.inputEnd( args ) ; 
			}	

			//Was open now false
			if ( hand.bOpen == false ) 
			{
				mousePressed( hand.screenPosition.x , hand.screenPosition.y , 0 ) ; 
				if ( mode == CAMERA_MOVEMENT )
					camera.inputBegin( args ) ; 
				if ( mode == DRAW_RIBBON ) 
					createAgents( true ) ;
			}
		}

		if ( mode == CAMERA_MOVEMENT ) //camera.getMouseInputEnabled() == true ) 
		{
			camera.inputDrag ( args ) ; 
		}
	}
	else
	{
		camera.inputEnd( args ) ; 
	}
	
	ofVec3f input = ofVec3f ( hand.screenPosition.x , ofGetHeight() - hand.screenPosition.y , hand.pos.z * 500.0f ) ;  
    ofVec3f offset = ofVec3f ( ofGetWidth() / 2 , ofGetHeight() /2  , 0 ) ;
    input -= offset ;

    for ( int a = 0 ; a < agents.size() ; a++ )
    {
        agents[a]->update( ) ;
        if ( mode == DRAW_RIBBON )  // && gui->isVisible() == false && camera.getMouseInputEnabled() == false )
        {
            agents[a]->followRibbon.addPoint( input ) ;
        }
    }
}

void testApp::gesutrePoseEvent ( int _gestureId ) 
{
	//Not the same gesture as last time ! 
	if ( gestureId != _gestureId ) 
	{
		if ( _gestureId == 2 ) 
			mode = CAMERA_MOVEMENT ;

		if ( _gestureId == 1 ) 
			mode = DRAW_RIBBON ; 
	}
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
						gesutrePoseEvent( gesture_list[i].id ) ; 
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
					float _curXRotation = ofMap( faceCentroid.x , 0.0f , 1.0f , -maxXRotation , maxXRotation ) ;
					float _curYRotation = ofMap( faceCentroid.y , 0.0f , 1.0f , -maxYRotation , maxYRotation ) ;
					Tweenzor::add( &curXRotation , curXRotation, _curXRotation , 0.0f , faceInterpolateTime , EASE_OUT_QUAD ) ;
					Tweenzor::add( &curYRotation , curYRotation, _curYRotation , 0.0f , faceInterpolateTime , EASE_OUT_QUAD ) ;
				}
			}

			//Update hands
			hand.updateFromPipeline( pipeline , mNode ) ; 

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

    ofQuaternion yRot( curXRotation , ofVec3f( 0,-1,0 ) );  
	ofQuaternion xRot( curYRotation , ofVec3f( -1,0,0 ) );  
	curRotation = yRot*xRot;
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
					ofBox( 25 ) ; 

					 //Draw the ribbons ! Nice and simple
					for ( int a = 0 ; a < agents.size() ; a++ )
					{
						agents[a]->draw( ) ;
					}

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
		//rgbTexture.draw( userRepArea ) ; 
	ofPopMatrix() ; 
	

	ofPushMatrix() ; 
		ofTranslate( userRepArea.x , userRepArea.y ) ; 
		hand.drawUserRep( ) ; 
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

	hand.drawCursor( ) ; 

	ofEnableAlphaBlending() ; 
	ofSetColor( 255 , 255 , 255 ) ; 

	ofPushMatrix() ; 
	ofTranslate( ofGetWidth() - 250 , -50 ) ; 
	switch ( mode ) 
	{
		
		case CAMERA_MOVEMENT :
			ofSetColor( 0 , 255 , 0 ) ;
			cameraIcon.draw ( 20 , ofGetHeight()/2 - cameraIcon.height/4 , cameraIcon.width /2 , cameraIcon.height/2 ) ; 
			ofSetColor( 255 , 255 , 255 ) ;
			drawIcon.draw ( 20 , ofGetHeight()/2 + drawIcon.height/4 , drawIcon.width / 2 , drawIcon.height / 2 ) ; 
			break ; 

		case DRAW_RIBBON :
			ofSetColor( 255 , 255 , 255 ) ;
			cameraIcon.draw ( 20 , ofGetHeight()/2 - cameraIcon.height/4 , cameraIcon.width /2 , cameraIcon.height/2 ) ; 
			ofSetColor( 0 , 255 , 0 ) ;
			drawIcon.draw ( 20 , ofGetHeight()/2 + drawIcon.height/4 , drawIcon.width / 2 , drawIcon.height / 2 ) ; 
			break ; 
	}
	ofPopMatrix( ) ; 
}

void testApp::keyPressed(int key){
	switch(key) {
		case 'F':
		case 'f':
			ofToggleFullscreen();
			break;

		case 'm':
        case 'M':
            cout << "toggeling mouse!" << endl ;
            //Toggle mouse on the camera

            if ( !camera.getMouseInputEnabled() )
                camera.enableMouseInput() ;
            else
                camera.disableMouseInput() ;
            break  ;

        case 'g':
        case 'G':
            gui->toggleVisible() ;
            break ;
	}
}


void testApp::setupUI ( )
{
    float dim = 24;
	float xInit = OFX_UI_GLOBAL_WIDGET_SPACING;
    float length = 180-xInit;

    drawPadding = false;

    gui = new ofxUICanvas(0, 0, length+xInit, ofGetHeight());

    //gui->addWidgetDown(new ofxUILabel("SLIDER WIDGETS", OFX_UI_FONT_LARGE));
    gui->addWidgetDown(new ofxUISlider(length-xInit,dim, 0.01, 9.0f , maxForce, "MAX FORCE" ));
    gui->addWidgetDown(new ofxUISlider(length-xInit,dim, 0.01, 3.0f , maxForceRandom, "MAX FORCE RANDOM" ));
    gui->addWidgetDown(new ofxUISlider(length-xInit,dim, 0.01, 50.0f , maxSpeed, "MAX SPEED" ));
    gui->addWidgetDown(new ofxUISlider(length-xInit,dim, 0.01, 15.0f , maxSpeedRandom, "MAX SPEED RANDOM" ));
    gui->addWidgetDown(new ofxUISlider(length-xInit,dim, 5.0f, 300.0f , bufferDistance, "BUFFER DISTANCE" ));

    gui->addWidgetDown(new ofxUISlider(length-xInit,dim, 1, 100 , tailLength, "TRAIL LENGTH" ));
    gui->addWidgetDown(new ofxUISlider(length-xInit,dim, 1, 100 , tailLengthRandom, "TRAIL LENGTH RANDOM" ));

    gui->addWidgetDown(new ofxUISlider(length-xInit,dim, 1, 100 , thickness, "THICKNESS" ));
    gui->addWidgetDown(new ofxUISlider(length-xInit,dim, 1, 100 , thicknessRandom, "THICKNESS RANDOM" ));

    gui->addWidgetDown(new ofxUISlider(length-xInit,dim, 2, 100 , numParticles, "NUM PARTICLES" ));

	gui->addSlider( "FACE INTERPOLATE TIME" , 0.1f , 1.5f , faceInterpolateTime , length-xInit , dim ) ; 
	gui->addSlider( "MAX X ROTATION" , -180.0f , 180.0f , maxXRotation , length-xInit , dim ) ; 
	gui->addSlider( "MAX Y ROTATION" , -180.0f , 180.0f , maxYRotation , length-xInit , dim ) ; 

    ofAddListener(gui->newGUIEvent,this,&testApp::guiEvent);

    gui->loadSettings("GUI/settings.xml") ;


   // gui->toggleVisible() ;
}


//--------------------------------------------------------------
void testApp::guiEvent(ofxUIEventArgs &e)
{
	string name = e.widget->getName();
	int kind = e.widget->getKind();

	if(name == "MAX FORCE")
	{
        ofxUISlider *slider = (ofxUISlider *) e.widget;
		maxForce = slider->getScaledValue();
        updateAgents( )  ;
    }

    if(name == "MAX FORCE RANDOM" )
	{
        ofxUISlider *slider = (ofxUISlider *) e.widget;
		maxForceRandom = slider->getScaledValue();
        updateAgents( )  ;
    }

    if(name == "MAX SPEED" )
	{
        ofxUISlider *slider = (ofxUISlider *) e.widget;
		maxSpeed = slider->getScaledValue();
        updateAgents( )  ;
    }

    if(name == "MAX SPEED RANDOM" )
	{
        ofxUISlider *slider = (ofxUISlider *) e.widget;
		maxSpeedRandom = slider->getScaledValue();
        updateAgents( )  ;
    }
    if ( name ==  "BUFFER DISTANCE"  )
    {
        ofxUISlider *slider = (ofxUISlider *) e.widget;
		bufferDistance = slider->getScaledValue();
        updateAgents( )  ;
    }

    if ( name == "TRAIL LENGTH" )
    {
        ofxUISlider *slider = (ofxUISlider *) e.widget;
        tailLength = slider->getScaledValue();
        updateAgentTrails( ) ;
    }

    if ( name == "TRAIL LENGTH RANDOM" )
    {
        ofxUISlider *slider = (ofxUISlider *) e.widget;
        tailLengthRandom = slider->getScaledValue();
        updateAgentTrails( ) ;
    }

    if ( name == "THICKNESS" )
    {
        ofxUISlider *slider = (ofxUISlider *) e.widget;
        thickness = slider->getScaledValue();
        updateAgentTrails ( ) ;
    }

    if ( name == "THICKNESS RANDOM" )
    {
        ofxUISlider *slider = (ofxUISlider *) e.widget;
        thicknessRandom = slider->getScaledValue();
        updateAgentTrails ( ) ;
    }

    if ( name == "NUM PARTICLES" )
    {
        ofxUISlider *slider = (ofxUISlider *) e.widget;
        numParticles = slider->getScaledValue();
        createAgents() ;
    }

	if ( name == "FACE INTERPOLATE TIME" )  faceInterpolateTime = ((ofxUISlider *) e.widget)->getScaledValue();
	if ( name == "MAX X ROTATION" )  maxXRotation = ((ofxUISlider *) e.widget)->getScaledValue();
	if ( name == "MAX Y ROTATION" )  maxYRotation = ((ofxUISlider *) e.widget)->getScaledValue();

    gui->saveSettings("GUI/settings.xml") ;

}

void testApp::updateAgentTrails( )
{
    //Trails are seperate from flocking parameters
    for ( int i = 0 ; i < agents.size() ; i++ )
    {
        //Only update if the random range will never make it go below 0
        if ( tailLengthRandom < tailLength )
            agents[i]->trailRibbon.tailLength = tailLength + ofRandom ( -tailLengthRandom , tailLengthRandom ) ;

        if ( thicknessRandom < thickness )
            agents[i]->trailRibbon.maxThickness = thickness + ofRandom ( -thicknessRandom , thicknessRandom ) ;
    }
}

void testApp::updateAgents( )
{
    if ( agents.size() > 0 )
    {
        //Update the flocking parameters
        for ( int i = 0 ; i < agents.size() ; i++ )
        {
            //Only update if the random range will never make it go below 0
            if ( maxForceRandom < maxForce )
                agents[i]->maxForce = maxForce + ofRandom( -maxForceRandom , maxForceRandom ) ;
            if ( maxSpeedRandom < maxSpeed )
                agents[i]->maxVelocity = maxSpeed + ofRandom( -maxSpeedRandom , maxSpeedRandom ) ;

            agents[i]->targetBufferDist = bufferDistance ;
        }
    }
}


void testApp::createAgents( bool bFirstMake )
{
    int oldNum = numRibbons ;
    numRibbons = numParticles ; //ofRandom ( numParticles ) ;

    if ( bFirstMake == true )
    {
        agents.clear() ;
        for ( int i = 0 ; i < numRibbons ; i++ )
        {
            Agent * agent = new Agent() ;
            agent->color = colorPool.getRandomColor( ) ;
            agent->setup( ofVec3f( 0 , 0, 0 ) ) ;
            agents.push_back( agent ) ;

        }

        updateAgents( ) ;
        updateAgentTrails( ) ;
        return ;
    }

    if ( agents.size() > 0 )
    {
        int diff = oldNum - numRibbons ;
        //cout << "agent # difference is : " << diff << endl ;
        // Diff > 0
        if ( oldNum > numRibbons )
        {
            //cout << "more last time than before! " << endl ;
            for ( int a = 0 ; a < diff ; a++ )
            {
                agents.pop_back() ;
            }
        }
        else
        {
            //cout << "less last time than before! " << endl ;
            int absDiff = abs( diff ) ;
            for ( int b = 0 ; b < absDiff ; b++ )
            {
                Agent * agent = new Agent() ;
                agent->color = colorPool.getRandomColor( ) ;
                agent->setup( ofVec3f( 0 , 0, 0 ) ) ;
                agent->followRibbon = agents[0]->followRibbon ;
                agents.push_back( agent ) ;
            }
        }
    }

    updateAgents( ) ;
	updateAgentTrails( ) ;
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

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

    if ( mode == CAMERA_MOVEMENT ) //|| gui->isVisible() == true )
        return ;

	if ( mode == ADJUST_COLOR ) 
		return ; 

    if ( gui->isEnabled() == true )
        return ;

    cout << "past GUI" << endl ;
    //Reset all of our ribbons
    bDrawPath = true ;

    numRibbons = 0 ;
    createAgents( true ) ;
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

	/*
    if ( mode == CAMERA_MOVEMENT ) //|| gui->isVisible() == true )
        return ;

	if ( mode == ADJUST_COLOR ) 
		return ; 
*/

    //End drawing the ribbon
    bDrawPath = false ;
}
