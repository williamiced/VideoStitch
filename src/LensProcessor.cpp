#include <header/LensProcessor.h>

void LensProcessor::undistort(Mat& frame) {
	GpuMat oriFrame(frame);
	GpuMat tmpFrame;
	cv::cuda::remap(oriFrame, tmpFrame, mUndistortMapX, mUndistortMapY, INTER_LINEAR);
	tmpFrame.download(frame);
}

LensProcessor::LensProcessor(vector<Mat> Ks, vector<Mat> Ds, Size videoSize, double focalLength) {
	// TODO: Use different [K, D] for different cameras
	mA = Ks[0];
	mD = Ds[0];

	// Refresh focal length from pto file
	mA.at<float>(0, 0) = static_cast<float> ( focalLength );
	mA.at<float>(1, 1) = static_cast<float> ( focalLength );
	mA.at<float>(0, 2) = static_cast<float> ( videoSize.width/2 );
	mA.at<float>(1, 2) = static_cast<float> ( videoSize.height/2 );

	Mat mapX, mapY;
	Rect newCamROI;
	
	initUndistortRectifyMap(mA, mD, Mat(), Mat(), videoSize, CV_32FC1, mapX, mapY);
	mUndistortMapX.upload(mapX);
	mUndistortMapY.upload(mapY);
}

LensProcessor::~LensProcessor() {
	
}
