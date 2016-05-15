#include <header/AlignProcessor.h>

using namespace cv;

void AlignProcessor::feed( int v, Mat frame ) {
	mFeededFrames[v] = frame;
}

void AlignProcessor::doAlign() {
	boost::timer::auto_cpu_timer boostTimer;
	if (mFF == nullptr) {
		//mFF = makePtr<SurfFeaturesFinder>();
		mFF = makePtr<OrbFeaturesFinder>();
	}
	for (int v=0; v<mViewCount; v++) {
		(*mFF) (mFeededFrames[v], mFeatures[v]);
		mFeatures[v].img_idx = v;
	}
	mFF->collectGarbage();
	
	BestOf2NearestMatcher matcher(true, 0.3f);
    matcher(mFeatures, mMatches);
    matcher.collectGarbage();

    HomographyBasedEstimator estimator;
    if (!estimator(mFeatures, mMatches, mCameras)) {
        cout << "Homography estimation failed.\n";
    } 
}

vector<Mat> AlignProcessor::getRotationMat() {
	vector<Mat> rotationMat( mCameras.size() );
	for (unsigned int i=0; i<mCameras.size(); i++) 
		mCameras[i].R.convertTo( rotationMat[i], CV_32F );
	return rotationMat;
}

vector<Mat> AlignProcessor::getIntrinsicMat() {
	vector<Mat> intrinsicMat( mCameras.size() );
	for (unsigned int i=0; i<mCameras.size(); i++)
		mCameras[i].K().convertTo( intrinsicMat[i], CV_32F );
	return intrinsicMat;
}

AlignProcessor::AlignProcessor(int viewCount) : mViewCount(viewCount) {
	mFeededFrames.resize(mViewCount);
	mFeatures.resize(mViewCount);
}

AlignProcessor::~AlignProcessor() {
	
}
