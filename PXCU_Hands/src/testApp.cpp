#include "testApp.h"
#define kMoveInc 10
#define kRotInc 5


const int HANDS = 2;
const int FINGERS = 5;
const PXCGesture::GeoNode::Label gHands[] = {PXCGesture::GeoNode::LABEL_BODY_HAND_LEFT,
											PXCGesture::GeoNode::LABEL_BODY_HAND_RIGHT};
																		
const PXCGesture::GeoNode::Label gFingers[] = {PXCGesture::GeoNode::LABEL_FINGER_THUMB,
												PXCGesture::GeoNode::LABEL_FINGER_INDEX,
												PXCGesture::GeoNode::LABEL_FINGER_MIDDLE,
												PXCGesture::GeoNode::LABEL_FINGER_RING,
												PXCGesture::GeoNode::LABEL_FINGER_PINKY};


//--------------------------------------------------------------
void testApp::setup()
{
	ofSetWindowShape(640*2,800);
	ofSetBackgroundColor(0);
	ofSetLogLevel(OF_LOG_NOTICE);
	ofSetVerticalSync(true);
	glEnable(GL_DEPTH_TEST);
	ofEnableLighting();
	light.enable();
	pathLines.setMode(OF_PRIMITIVE_LINE_STRIP);

	camFixed = false;
	//PXC
	mSession = PXCUPipeline_Create();

	
	if(!PXCUPipeline_Init(mSession, (PXCUPipeline)(PXCU_PIPELINE_COLOR_VGA|PXCU_PIPELINE_DEPTH_QVGA|PXCU_PIPELINE_GESTURE)))
	{
		ofLogNotice() << "Unable to initialize session" << endl;
		return;
	}

	if(PXCUPipeline_QueryRGBSize(mSession, &mCW, &mCH))
	{

		mRGBMap = new unsigned char[mCW*mCH*4];
		mRGBTexture.allocate(mCW,mCH,GL_RGBA);
	}
	if(PXCUPipeline_QueryLabelMapSize(mSession, &mLW, &mLH))
	{
		mLabelMap = new unsigned char[mLW*mLH];
		mLabelTexture.allocate(mLW,mLH,GL_LUMINANCE);
	}
	if(PXCUPipeline_QueryDepthMapSize(mSession, &mLW, &mLH))
	{
		mDepthBuffer = new short[mLW*mLH];
		mDepthMap = new unsigned char[mLW*mLH];
		mDepthTexture.allocate(mLW,mLH,GL_LUMINANCE);
	}
	if(PXCUPipeline_QueryIRMapSize(mSession, &mLW, &mLH))
	{
		mIRBuffer = new short[mLW*mLH];
		mIRMap = new unsigned char[mLW*mLH];
		mIRTexture.allocate(mLW,mLH,GL_LUMINANCE);
	}

	//cam
	cam.resetTransform();
	cam.setFov(60);
	cam.clearParent();
	cam.setPosition(0, 10, 120);
	cam.setParent(hero);
	camSwith = false;
	frc.set(0,0,0);
	//node 
	float freqMult = 1;
	float amp = 30;	
	float scale = 1;
	hero_Pos.set(0,0,0);
	hero.setPosition(ofVec3f(sin(ofGetElapsedTimef() * freqMult) * amp, cos(ofGetElapsedTimef() * freqMult) * amp, sin(ofGetElapsedTimef() * freqMult * 0.7) * amp));
	hero.setOrientation(ofPoint(0,0,0));
	hero.setScale(scale);
	foc.set(0,0,0);

	for (int i = 0; i < 600; i ++)
	{
		freqMult = ofRandom(0.5,2);
		amp = ofRandom(10,30);	
		scale = ofRandom(1,15);
		ofNode temNode;
		temNode.setPosition(ofRandom(-1000,1000),ofRandom(-1000,1000),ofRandom(-50000,0));
		temNode.setOrientation(ofVec3f(sin(ofGetElapsedTimef() * freqMult * 0.2) * amp * 5, cos(ofGetElapsedTimef() * freqMult * 0.2) * amp * 5, sin(ofGetElapsedTimef() * freqMult * 0.2 * 0.7) * amp * 5));
		temNode.setScale(scale);
		nodes.push_back(temNode);
	}
}

//--------------------------------------------------------------
void testApp::update()
{
	if(PXCUPipeline_AcquireFrame(mSession, false))
	{
		mPositions.clear();

		if(PXCUPipeline_QueryLabelMap(mSession, mLabelMap, 0))
			mLabelTexture.loadData(mLabelMap, mLW, mLH, GL_LUMINANCE);
		
		for(int i=0;i<HANDS;i++)
		{
			for(int j=0;j<FINGERS;j++)
			{
				if(PXCUPipeline_QueryGeoNode(mSession, gHands[i]|gFingers[j], &mNode)){
					myPositions tempPos;
					tempPos.imagePos.set(mNode.positionImage.x*2, mNode.positionImage.y*2);
					tempPos.worldPos.set(mNode.positionWorld.x,mNode.positionWorld.y,mNode.positionWorld.z);
					tempPos.node = gFingers[j];
					mPositions.push_back(tempPos);
				}

			if(PXCUPipeline_QueryGeoNode(mSession, gHands[i], &mNode)){
				myPositions tempPos;
				tempPos.imagePos.set(mNode.positionImage.x*2, mNode.positionImage.y*2);
				tempPos.worldPos.set(mNode.positionWorld.x,mNode.positionWorld.y,mNode.positionWorld.z);
				tempPos.node = gHands[i];
				mPositions.push_back(tempPos);
				}
			}
		}

		//video
		if(PXCUPipeline_QueryRGB(mSession, mRGBMap))
			mRGBTexture.loadData(mRGBMap, mCW, mCH, GL_RGBA);

		bool getDepth = PXCUPipeline_QueryDepthMap(mSession, mDepthBuffer);
		bool getIR = PXCUPipeline_QueryIRMap(mSession, mIRBuffer);

		if(getDepth||getIR)
		{
			for(int i=0;i<mLW*mLH;++i)
			{	
				if(getDepth)
					mDepthMap[i] = (unsigned char)ofMap((float)mDepthBuffer[i],0,2000,0,255);
				if(getIR)
					mIRMap[i] = (unsigned char)ofMap((float)mIRBuffer[i],0,3000,0,255);
			}
			if(getDepth)
				mDepthTexture.loadData(mDepthMap, mLW, mLH, GL_LUMINANCE);
			if(getIR)
				mIRTexture.loadData(mIRMap, mLW, mLH, GL_LUMINANCE);
		}

	
		
		PXCUPipeline_ReleaseFrame(mSession);
	}


	//my------------------------------
	L.set(0,0,0);
	R.set(0,0,0);
	bool bLeftHand,bRightHand, left, right;
	bLeftHand = false,
	bRightHand = false;
	left = false;
	right = false;
	float dis;

	for(int i = 0; i<mPositions.size();i++){
		if(mPositions[i].node == PXCGesture::GeoNode::LABEL_BODY_HAND_LEFT){
			L = mPositions[i].worldPos;
			bLeftHand = true;
		}
		else if(mPositions[i].node == PXCGesture::GeoNode::LABEL_BODY_HAND_RIGHT){
			R =  mPositions[i].worldPos;
			bRightHand = true;
		}
	}
	
	if (bLeftHand&&bRightHand)
	{
		dis = L.distance(R);

		cout<<dis<<endl;
		

		if (L.y - R.y>0.02 && dis<0.3)
		{
		//	cout<<"left"<<endl;
			left = true;
			right = false;
		}else if(R.y-L.y>0.02 && dis<0.3){
			cout<<"right"<<endl;
			left = false;
			right = true;
		}
		
		else{
			left = false;
			right = false;
		}

// 		if (L.y - R.y>0.02 && dis>0.3)
// 		{
// 			
// 			hero_Pos.y += 30;
// 		}
// 		
// 		if (R.y-L.y>0.02 && dis>0.3)
// 		{
// 			
// 			hero_Pos.y -= 30;
// 		}

		
	}
	
	if (left)
	{
		frc.x +=0.01;
		if (frc.x > 1)
		{
			frc.x = 1;
		}
		
	}else if(right)
	{
		frc.x -=0.01;
		if (frc.x < -1)
		{
			frc.x = -1;
		}
	}else{
		if (frc.x > 0)
		{	frc.x-=0.3;
		}else if(frc.x < 0){
			frc.x+=0.3;
		}else{
			frc.x = 0;
		}
	}
	
	hero_Pos.z -= 100*(1-frc.x);
	hero_Pos.x += 100*frc.x;
	

	       
	
	float freqMult = 1;
	float amp = 30;
	float scale = 1;
	
	
	
	//hero.setPosition(ofVec3f(sin(ofGetElapsedTimef() * freqMult) * amp, cos(ofGetElapsedTimef() * freqMult) * amp, sin(ofGetElapsedTimef() * freqMult * 0.7) * amp));
	hero_Pos += foc;
	
	hero.setPosition(hero_Pos);
	hero.setScale(scale);
	hero.setOrientation(frc*10);

	for (int i=0; i<nodes.size(); i++)
	{
		float freqMult = 1;
		float amp = 30;	
		nodes[i].setOrientation(ofVec3f(sin(ofGetElapsedTimef() * freqMult * 0.2) * amp * 5, cos(ofGetElapsedTimef() * freqMult * 0.2) * amp * 5, sin(ofGetElapsedTimef() * freqMult * 0.2 * 0.7) * amp * 5));
	}
	
	pre_L = L;
	pre_R = R;
}

//--------------------------------------------------------------
void testApp::draw()
{
	
	ofBackgroundGradient(60, 0);
	ofSetLineWidth(2);

	//PXC
	if (camFixed)
	{
		glDisable(GL_CULL_FACE);
		ofDisableLighting();
		glDisable(GL_DEPTH_TEST);
		ofSetColor(255);
		mLabelTexture.draw(0,0,640,480);

		ofPushStyle();

		ofFill();
		//for(vector<ofPoint>::iterator vit=mPositions.begin();vit!=mPositions.end();++vit){

			for(int i = 0; i<mPositions.size();i++){
				if(mPositions[i].node == PXCGesture::GeoNode::LABEL_FINGER_THUMB){
					ofSetColor(ofColor::red);
					ofCircle(mPositions[i].imagePos.x,mPositions[i].imagePos.y, 15);
				
				}
				else if(mPositions[i].node == PXCGesture::GeoNode::LABEL_FINGER_INDEX){
					ofSetColor(ofColor::yellow);
					ofCircle(mPositions[i].imagePos.x,mPositions[i].imagePos.y, 15);
				}
				else if(mPositions[i].node == PXCGesture::GeoNode::LABEL_FINGER_MIDDLE){
					ofSetColor(ofColor::blue);
					ofCircle(mPositions[i].imagePos.x,mPositions[i].imagePos.y, 15);
				 }

				else if(mPositions[i].node == PXCGesture::GeoNode::LABEL_FINGER_PINKY){
					 ofSetColor(ofColor::green);
					 ofCircle(mPositions[i].imagePos.x,mPositions[i].imagePos.y, 15);
				 }

				else if(mPositions[i].node == PXCGesture::GeoNode::LABEL_FINGER_RING){
					ofSetColor(ofColor::pink);
					ofCircle(mPositions[i].imagePos.x,mPositions[i].imagePos.y, 15);
					
				 }
				else if(mPositions[i].node == PXCGesture::GeoNode::LABEL_BODY_HAND_LEFT){
					ofSetColor(ofColor::white);
					float z = ofMap(mPositions[i].worldPos.z,0,0.1,0,40,true);
					ofCircle(mPositions[i].imagePos.x,mPositions[i].imagePos.y, 40-z);
					ofSetColor(60,60,60);
					ofDrawBitmapString(ofToString(z)+"   "+ofToString(mPositions[i].worldPos.z),100,100);
				}
				else if(mPositions[i].node == PXCGesture::GeoNode::LABEL_BODY_HAND_RIGHT){
					ofSetColor(ofColor::burlyWood);
					float z = ofMap(mPositions[i].worldPos.z,0,0.1,0,40,true);
					ofCircle(mPositions[i].imagePos.x,mPositions[i].imagePos.y, 40-z);
					ofSetColor(60,60,60);
					ofDrawBitmapString(ofToString(z)+"   "+ofToString(mPositions[i].worldPos.z),100,130);
				}

				

		}
		ofPopStyle();


		mRGBTexture.draw(0+640,0,320,240);
		mLabelTexture.draw(640+320,0,320,240);
		mDepthTexture.draw(640,240,320,240);
		mIRTexture.draw(640+320,240,320,240);
	}else{

		if(ofGetMousePressed(0)) {
			static float lon = 0;
			static float lat = 0;

			lon = ofClamp(lon + mouseX - ofGetPreviousMouseX(), -180, 180);
			lat = ofClamp(lat + mouseY - ofGetPreviousMouseY(), -90, 90);

			cam.orbit(lon, lat, 100, hero);
			
		}

		glEnable(GL_CULL_FACE);
		ofEnableLighting();

		if (!camSwith)
		{cam.lookAt(hero);
		cam.begin();
		ofDrawAxis(5000);
		hero.draw();
		for (int i =0; i<nodes.size(); i++)
		{
			nodes[i].draw();
		}
		cam.end();
		}else{
			hero.begin();
			for (int i =0; i<nodes.size(); i++)
			{
				nodes[i].draw();
			}
			
			ofDrawAxis(5000);
			hero.end();
		}
		
		
	}
	
}

//--------------------------------------------------------------
void testApp::exit()
{
	PXCUPipeline_Close(mSession);
}
//--------------------------------------------------------------
void testApp::keyPressed(int key){
	ofNode *n = &cam;

	switch(key) {
	case 'c':
		camFixed = !camFixed;
		break;
	case OF_KEY_LEFT: n->pan(kRotInc); break;
	case OF_KEY_RIGHT: n->pan(-kRotInc); break;

	case OF_KEY_UP: n->tilt(-kRotInc); break;
	case OF_KEY_DOWN: n->tilt(kRotInc); break;

	case ',': n->roll(kRotInc); break;
	case '.': n->roll(-kRotInc); break;

	case 'a': n->truck(-kMoveInc); break;
	case 'd': n->truck(kMoveInc); break;

	case 'w': n->dolly(-kMoveInc); break;
	case 's': n->dolly(kMoveInc); break;

	case 'r': n->boom(kMoveInc); break;
	case 'f': n->boom(-kMoveInc); break;
	case 'g': 
		camSwith = !camSwith;
		break;
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}