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
	Rect newCamROI;
	
	/*
	initUndistortRectifyMap(mA, mD, Mat(), Mat(), videoSize, CV_32FC1, mapX, mapY);
	mUndistortMapX.upload(mapX);
	mUndistortMapY.upload(mapY);
	*/
}

LensProcessor::~LensProcessor() {
	
}
