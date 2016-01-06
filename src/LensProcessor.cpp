#include <header/LensProcessor.h>

void LensProcessor::undistort(Mat& frame) {
	GpuMat oriFrame(frame);
	GpuMat tmpFrame;
	cv::cuda::remap(oriFrame, tmpFrame, mUndistortMapX, mUndistortMapY, INTER_LINEAR);
	tmpFrame.download(frame);
}

LensProcessor::LensProcessor(map<string, Mat> calibrationData, Size videoSize) {
	mA = calibrationData["cameraMatA"];
	mD = calibrationData["distCoeffs"];

	Mat mapX, mapY;
	initUndistortRectifyMap(mA, mD, Mat::eye(3, 3, CV_32F), mA, videoSize, CV_32FC1, mapX, mapY);
	mUndistortMapX.upload(mapX);
	mUndistortMapY.upload(mapY);
}

LensProcessor::~LensProcessor() {
	
}