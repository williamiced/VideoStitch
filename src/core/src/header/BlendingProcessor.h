#ifndef _H_BLENDING_PROCESSOR
#define _H_BLENDING_PROCESSOR

#include <header/Params.h>
#include <header/Usage.h>
#include <boost/timer/timer.hpp>
#include <omp.h>
#include "opencv2/stitching/detail/blenders.hpp"
#include "opencv2/stitching/detail/util.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/core/cuda.hpp"

using namespace cv::cuda;

class BlendingProcessor : public cv::detail::Blender {
	private:
		int mViewCount;
		cv::Rect mCanvasROI;
		vector<cv::Mat> mDilateMasks;
		vector<cv::Mat> mWeightMaps;

		// New approach to speed up
		vector<cv::Mat> mAdjustWeightMaps;
		vector<cv::Mat> mFinalAdjustWeightMaps;

	private:
		float sharpness_;
    	cv::UMat dst_weight_map_;

	public:
		void prepare(cv::Rect dst_roi);
		void feed (cv::InputArray _img, cv::InputArray mask, cv::Point tl);
		void feeds(vector<cv::Mat> imgs);
		void blend(cv::InputOutputArray dst, cv::InputOutputArray dst_mask);
		void setSharpness(float val) { sharpness_ = val; }
		void genWeightMapByMasks(vector<cv::Mat> masks);
		void preProcess(cv::Rect dst_roi, vector<cv::Mat> imgs);
		void newPreprocess();
		void genFinalMap(vector<double> gains);
		void getFinalMap(vector<cv::Mat>& finalMap);

		BlendingProcessor( int vc, cv::Rect canvasROI );
		~BlendingProcessor();
};

#endif // _H_BLENDING_PROCESSOR
