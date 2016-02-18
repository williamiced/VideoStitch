#ifndef _H_ALIGN_PROCESSOR
#define _H_ALIGN_PROCESSOR

#include <header/Params.h>
#include <header/Usage.h>
#include <boost/timer/timer.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/stitching/detail/matchers.hpp"
#include "opencv2/stitching/detail/camera.hpp"
#include "opencv2/stitching/detail/motion_estimators.hpp"
#include "opencv2/core/cuda.hpp"

using namespace cv;
using namespace cv::detail;
using namespace cv::cuda;

class AlignProcessor {
	private:
		int mViewCount;
		vector<Mat> mFeededFrames;
		vector<ImageFeatures> mFeatures;
		vector<MatchesInfo> mMatches;
		vector<CameraParams> mCameras;
		Ptr<FeaturesFinder> mFF; 
		
	public:
		vector<Mat> getRotationMat();
		vector<Mat> getIntrinsicMat();
		void feed( int v, Mat frame );
		void doAlign();
		AlignProcessor(int viewCount);
		~AlignProcessor();
};

#endif // _H_ALIGN_PROCESSOR
