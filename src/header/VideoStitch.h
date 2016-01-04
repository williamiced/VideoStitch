#ifndef _H_VIDEO_STITCH
#define _H_VIDEO_STITCH

#include <iostream>
#include <algorithm>
#include <ctime> 
#include <header/VideoLoader.h>
#include <header/LensProcessor.h>
#include <header/VideoStablizer.h>
#include <header/ExposureProcessor.h>
#include <header/Usage.h>

#include "opencv2/core/cuda.hpp"
#include "opencv2/cudawarping.hpp"

using namespace cv::cuda;
using namespace std;

class VideoStitcher {
	private:
		VideoLoader* mVL;
		LensProcessor* mLP;
		VideoStablizer* mVS;
		ExposureProcessor* mEP;
		vector<Mat> mProjMat;

	public:
		void calcProjectionMatrix();
		void projectOnCanvas(GpuMat& canvas, Mat frame, int vIdx);
		void doRealTimeStitching(int argc, char* argv[]);
		VideoStitcher(int argc, char* argv[]);
		~VideoStitcher();
};

#endif // _H_VIDEO_STITCH