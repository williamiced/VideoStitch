#ifndef _H_EXPOSURE_PROCESSOR
#define _H_EXPOSURE_PROCESSOR

#include <header/Params.h>
#include <header/Usage.h>
#include <omp.h>
#include <boost/timer/timer.hpp>
#include "opencv2/stitching/detail/exposure_compensate.hpp"
#include "opencv2/stitching/detail/util.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/core/cuda.hpp"
#include "opencv2/cudaarithm.hpp"

#include <typeinfo>

using namespace cv::cuda;

class ExposureProcessor : public cv::detail::ExposureCompensator {
	private:
		vector<cv::Point> mCorners;
		int mViewCount;
		bool mNeedFeed;
		cv::Mat_<double> gains_;

	public:
		void feed(const std::vector<cv::Point> &corners, const std::vector<cv::UMat> &images,
                               const std::vector<cv::UMat> &masks);
		void feed(const std::vector<cv::Point> &corners, const std::vector<cv::UMat> &images,
              const std::vector<std::pair<cv::UMat,uchar> > &masks);
    	void apply(int index, cv::Point corner, cv::InputOutputArray image, cv::InputArray mask);
    	std::vector<double> gains() const;
		void feedExposures(vector<cv::Mat> warpedImg, vector<cv::Mat> warpedMasks);
		void doExposureCompensate(vector<cv::Mat> warpedImg, vector<cv::Mat> warpedMasks, cv::Rect renderArea);
		bool needFeed();
		ExposureProcessor( vector<cv::Point> c, int vc);
		~ExposureProcessor();
};

#endif // _H_EXPOSURE_PROCESSOR
