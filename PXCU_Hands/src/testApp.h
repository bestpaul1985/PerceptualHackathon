#ifndef TEST_APP_GUARD
#define TEST_APP_GUARD

#include "ofMain.h"
#include "pxcupipeline.h"
#include "myPositions.h"

class testApp : public ofBaseApp
{
public:
	void setup();
	void update();
	void draw();
	void exit();
	void keyPressed (int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg); 

	bool camFixed,camSwith;
	ofLight light;
	ofCamera cam;
	ofCamera hero;
	ofPoint L,R,pre_L, pre_R, hero_Pos;
	ofPoint foc,frc;
	ofMesh pathLines;
	ofMesh mesh;
	vector<ofNode> nodes;
	ofImage bgImg;
private:
	int  mCW, mCH, mLW, mLH;
	unsigned char *mRGBMap, *mLabelMap, *mIRMap, *mDepthMap;
	short *mIRBuffer, *mDepthBuffer;

	ofTexture mRGBTexture;
	ofTexture mLabelTexture;
	ofTexture mDepthTexture;
	ofTexture mIRTexture;

	//vector<ofPoint> mPositions;

	PXCUPipeline_Instance mSession;
	PXCGesture::GeoNode mNode;

	vector<myPositions> mPositions;
};

#endif