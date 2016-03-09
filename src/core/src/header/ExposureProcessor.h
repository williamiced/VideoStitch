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
		vector<Point> mCorners;
		int mViewCount;
		bool mNeedFeed;
		Mat_<double> gains_;

	public:
		void feed(const std::vector<Point> &corners, const std::vector<UMat> &images,
                               const std::vector<UMat> &masks);
		void feed(const std::vector<Point> &corners, const std::vector<UMat> &images,
              const std::vector<std::pair<UMat,uchar> > &masks);
    	void apply(int index, Point corner, InputOutputArray image, InputArray mask);
    	std::vector<double> gains() const;
		void feedExposures(vector<Mat> warpedImg, vector<Mat> warpedMasks);
		void doExposureCompensate(vector<Mat> warpedImg, vector<Mat> warpedMasks, Rect renderArea);
		bool needFeed();
		ExposureProcessor( vector<Point> c, int vc);
		~ExposureProcessor();
};

#endif // _H_EXPOSURE_PROCESSOR
