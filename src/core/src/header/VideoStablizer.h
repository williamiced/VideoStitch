#ifndef _H_VIDEO_STABLIZER
#define _H_VIDEO_STABLIZER

#include <header/VideoLoader.h>
#include <header/Params.h>
#include <header/Usage.h>
#include "opencv2/core/core.hpp"
#include "opencv2/core/cuda.hpp"

using namespace cv::cuda;

class VideoStablizer {
	private:

	public:
		void stablize(GpuMat& canvas);
		VideoStablizer();
		~VideoStablizer();
};

#endif // _H_VIDEO_STABLIZER
