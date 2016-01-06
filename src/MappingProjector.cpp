#include <header/MappingProjector.h>

void MappingProjector::calcProjectionMatrix(map< string, Mat > calibrationData) {
	int seqCount = 6;
	for (int v=0; v<seqCount; v++) 
		mProjMat.push_back(Mat::eye(2, 3, CV_32F));

	mA = calibrationData["cameraMatA"];
	mRT = Mat(3, 4, CV_32F);
	/** 
		[TODO]
			Use calibration file to get the proejction matrix
	*/
}

void MappingProjector::projectOnCanvas(GpuMat& canvas, Mat frame, int vIdx) {
	GpuMat newFrame;
	GpuMat oriFrame(frame);
	cv::cuda::warpAffine(oriFrame, newFrame, mProjMat[vIdx], Size(frame.cols, frame.rows));
	
	// [TODO] Currently copyTo is a bottleneck which reduce fps from 40 to 13
	// newFrame.copyTo( canvas );
	/** 
		[TODO]
			Paste new frame onto canvas
	*/
}

MappingProjector::MappingProjector() {
	
}

MappingProjector::~MappingProjector() {
	
}