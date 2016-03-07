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
		Rect mCanvasROI;
		vector<Mat> mDilateMasks;
		vector<Mat> mWeightMaps;

	private:
		float sharpness_;
    	UMat dst_weight_map_;

	public:
		void prepare(Rect dst_roi);
		void feed (InputArray _img, InputArray mask, Point tl);
		void feeds(vector<Mat> imgs);
		void blend(InputOutputArray dst, InputOutputArray dst_mask);
		void setSharpness(float val) { sharpness_ = val; }
		void genWeightMapByMasks(vector<Mat> masks);
		void preProcess(Rect dst_roi, vector<Mat> imgs);

		BlendingProcessor( int vc, Rect canvasROI );
		~BlendingProcessor();
};

#endif // _H_BLENDING_PROCESSOR
