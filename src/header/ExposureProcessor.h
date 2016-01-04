#ifndef _H_EXPOSURE_PROCESSOR
#define _H_EXPOSURE_PROCESSOR

#include <header/VideoLoader.h>
#include <header/Params.h>
#include <header/Usage.h>
#include "opencv2/core/core.hpp"
#include "opencv2/core/cuda.hpp"

using namespace cv::cuda;

class ExposureProcessor {
	private:

	public:
		void exposureBlending(GpuMat& canvas);
		ExposureProcessor();
		~ExposureProcessor();
};

#endif // _H_EXPOSURE_PROCESSOR
