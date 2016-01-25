#include <header/ExposureProcessor.h>

void ExposureProcessor::doExposureCompensate( vector<Mat> warpedImg ) {
	for (int v=0; v<mViewCount; v++) 
		mEC->apply(v, mCorners[v], warpedImg[v], mMasks[v]);
}

void ExposureProcessor::feedExposures(vector<Mat> warpedImg) {
	vector<UMat> imgs, masks;
	for (int v=0; v<mViewCount; v++) {
		imgs.push_back(warpedImg[v].getUMat(ACCESS_RW));
		masks.push_back(mMasks[v].getUMat(ACCESS_RW));
	}
	mEC->feed(mCorners, imgs, masks);
}

ExposureProcessor::ExposureProcessor( vector<Point> c, vector<Mat> m, int vc) : mCorners(c), mMasks(m), mViewCount(vc) {
	mEC = cv::detail::ExposureCompensator::createDefault(cv::detail::ExposureCompensator::GAIN);
}

ExposureProcessor::~ExposureProcessor() {
	
}