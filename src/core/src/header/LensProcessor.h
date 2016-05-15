#ifndef _H_LENS_PROCESSOR
#define _H_LENS_PROCESSOR

#include <header/VideoLoader.h>
#include <header/Params.h>
#include <header/Usage.h>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "opencv2/core/cuda.hpp"
#include "opencv2/cudawarping.hpp"

using namespace cv::cuda;

class LensProcessor {
	private:
		cv::Mat mA;
		cv::Mat mD;
		GpuMat mUndistortMapX;
		GpuMat mUndistortMapY;

	public:
		void undistort(cv::Mat& frame);
		LensProcessor(vector<cv::Mat> Ks, vector<cv::Mat> Ds, cv::Size videoSize, double focalLength);
		~LensProcessor();
};

#endif // _H_LENS_PROCESSOR
