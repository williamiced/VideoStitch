#ifndef _H_EXPOSURE_PROCESSOR
#define _H_EXPOSURE_PROCESSOR

#include <header/Params.h>
#include <header/Usage.h>
#include "opencv2/stitching/detail/exposure_compensate.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/core/cuda.hpp"

using namespace cv::cuda;

class ExposureProcessor {
	private:
		cv::Ptr<cv::detail::ExposureCompensator> mEC;
		vector<Point> mCorners;
		vector<Mat> mMasks;
		int mViewCount;

	public:
		void feedExposures(vector<Mat> warpedImg);
		void doExposureCompensate(vector<Mat> warpedImg);
		ExposureProcessor( vector<Point> c, vector<Mat> m, int vc);
		~ExposureProcessor();
};

#endif // _H_EXPOSURE_PROCESSOR
