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
		int mViewCount;
		bool mNeedFeed;

	public:
		void feedExposures(vector<Mat> warpedImg, vector<Mat> warpedMasks);
		void doExposureCompensate(vector<Mat> warpedImg, vector<Mat> warpedMasks);
		bool needFeed();
		ExposureProcessor( vector<Point> c, int vc);
		~ExposureProcessor();
};

#endif // _H_EXPOSURE_PROCESSOR
