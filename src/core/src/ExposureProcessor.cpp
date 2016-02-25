#include <header/ExposureProcessor.h>

void ExposureProcessor::doExposureCompensate( vector<Mat> warpedImg, vector<Mat> warpedMasks) {
	for (int v=0; v<mViewCount; v++) 
		mEC->apply(v, mCorners[v], warpedImg[v], warpedMasks[v]);
}

void ExposureProcessor::feedExposures(vector<Mat> warpedImg, vector<Mat> warpedMasks) {
	vector<UMat> imgs, masks;
	for (int v=0; v<mViewCount; v++) {
		imgs.push_back(warpedImg[v].getUMat(ACCESS_RW));
		masks.push_back(warpedMasks[v].getUMat(ACCESS_RW));
	}
	mEC->feed(mCorners, imgs, masks);
	mNeedFeed = false;
}

bool ExposureProcessor::needFeed() {
	return mNeedFeed;
}

ExposureProcessor::ExposureProcessor( vector<Point> c, int vc) : mCorners(c), mViewCount(vc) {
	mEC = cv::detail::ExposureCompensator::createDefault(cv::detail::ExposureCompensator::GAIN);
	mNeedFeed = true;
}

ExposureProcessor::~ExposureProcessor() {
	
}