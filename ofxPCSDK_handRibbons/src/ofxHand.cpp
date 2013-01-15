#include "ofxHand.h"

//Enumerators and Labels
PXCGesture::GeoNode::Label gFingers[] = {	PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY|PXCGesture::GeoNode::LABEL_FINGER_THUMB,
											PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY|PXCGesture::GeoNode::LABEL_FINGER_INDEX,
											PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY|PXCGesture::GeoNode::LABEL_FINGER_MIDDLE,
											PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY|PXCGesture::GeoNode::LABEL_FINGER_RING,
											PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY|PXCGesture::GeoNode::LABEL_FINGER_PINKY };


PXCGesture::GeoNode::Label gHands[] = {		PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY,
											PXCGesture::GeoNode::LABEL_BODY_HAND_SECONDARY };



void ofxHand::setup( int _handIndex , float _inputWidth , float _inputHeight , bool _bMirrorX , bool _bMirrorY ) 
{
	handIndex = _handIndex ; 
	inputWidth = _inputWidth ; 
	inputHeight = _inputHeight ; 
	bExists = false;
	geoLabel = gHands[ handIndex ] ; 
	pos.set( 0 , 0 , 0 ) ; 
	bMirrorX = _bMirrorX ; 
	bMirrorY = _bMirrorY ; 

	//Setup the fingers
	for(int i=0;i<NFINGERS;++i)
	{	
		fingers[i].bExists = false;
		fingers[i].geoLabel = gFingers[i];
		fingers[i].pos.set(0,0,0);
	}

	openThreshold = 0.5f ; 
	bOpen = false ; 
}

void ofxHand::update( ) 
{
	float openness = mNode.openness * .01 ; 
	if ( openness > openThreshold ) 
	{
		if ( bOpen == false ) 
			bOpen = true ; 
	}
	else
	{
		if ( bOpen == true ) 
			bOpen = false ; 
	}
}

void ofxHand::updateFromPipeline ( PXCUPipeline_Instance pipeline , PXCGesture::GeoNode _mNode ) 
{
	mNode = _mNode ; 
	//Update all the fingers
	int nFingersDetected = 0;
	for(int i=0;i<NFINGERS;++i)
	{
		fingers[i].bExists = PXCUPipeline_QueryGeoNode( pipeline , fingers[i].geoLabel, &mNode);
		if(fingers[i].bExists ) 
		{	
			nFingersDetected ++;
			float _x =  mNode.positionImage.x ; 
			float _y =  mNode.positionImage.y ; 
			float _z =  mNode.positionImage.z ; 
			if ( bMirrorX ) 
				_x = inputWidth - mNode.positionImage.x ; 
			if ( bMirrorY ) 
				_y = inputHeight - mNode.positionImage.y ; 
			
			fingers[i].pos.set( _x , _y , _z );
		} 
	}

	//Update hands
	//for(int i=0; i <2; i++)
	//{
		bExists = PXCUPipeline_QueryGeoNode( pipeline , geoLabel, &mNode );
		if( bExists)
		{
			float _x = mNode.positionImage.x ; 
			float _y = mNode.positionImage.y ; 
			if ( bMirrorX ) 
				_x = inputWidth - mNode.positionImage.x ; 
			if ( bMirrorY ) 
				_y = inputHeight - mNode.positionImage.y ; 
			pos.set( _x , _y , mNode.openness*.01);

			//Normalize the positions between 0 and 1 based on the picture
			//then offset it so that the range is -0.5 to 0.5 
			_x = ( _x / inputWidth ) - 0.5 ; 
			_y = ( _y / inputHeight ) - 0.5 ; 

			//Scale the coordinates to make it easier to reach the edges.
			float sensitivity = 4.0f ; 
			_x *= sensitivity ; 
			_y *= sensitivity ; 
			ofPoint center = ofPoint ( ofGetWidth() / 2 , ofGetHeight() / 2 ) ; 
			_x = center.x + _x * ( ofGetWidth() / 2 ) ; 
			_y = center.y + _y * ( ofGetHeight() / 2 ) ; 

			//Limit the X and Y to just the screen
			float padding = 50 ; 
			float maxX = ofGetWidth() - 50 ; 
			float maxY = ofGetHeight() - 50 ; 
			if ( _x < padding ) _x = padding ; 
			if ( _x > maxX ) _x = maxX ; 
			if ( _y < padding ) _y = padding ; 
			if ( _y > maxY ) _y = maxY ; 
			screenPosition.x = _x ; 
			screenPosition.y = _y ; 
			
		//}
	}
}


void ofxHand::drawCursor( ) 
{
	ofPushStyle() ; 
		if ( bOpen == false ) 
			ofFill() ; 
		else
			ofNoFill () ; 
		//cout << "screenPosition : " << screenPosition.x << " , " << screenPosition.y << endl ; 
		ofSetColor( 255 , 255 , 0 );
		ofCircle( screenPosition ,  15 + 35 *  mNode.openness*.01 );
	ofPopStyle() ; 
}

void ofxHand::drawUserRep( ) 
{
	//Draw Fingers
	ofPushMatrix( ) ; 
		ofPushStyle( ) ; 
		for(int i=0;i<NFINGERS;++i)
		{
			if( fingers[i].bExists ) 
			{	
				ofNoFill( ) ; 
				ofSetLineWidth( 2 ) ; 
				ofEnableSmoothing() ; 
				ofSetColor( 255 , 0 , 255 ) ;
				ofCircle( fingers[i].pos , 7 ) ; 
			} 
		}
		ofPopStyle( ) ; 
	ofPopMatrix() ; 

	if( bExists )
	{
		ofPushStyle() ; 
			ofEnableSmoothing() ; 
			ofSetColor( 255 , 0 , 255 );
			ofNoFill();
			ofCircle( pos,  5 + 15 *  mNode.openness*.01 );
			ofFill();
			ofCircle( pos,  5 + 15 *  mNode.openness*.01 );
		ofPopStyle() ;
	}
}

