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
		vector<Point> mCorners;
		vector<Size> mSizes;
		vector<Mat> mDilateMasks;

	private:
		float sharpness_;
    	UMat weight_map_;
    	UMat dst_weight_map_;

	public:
		void doBlending( vector<Mat> warpedImg, Mat& result, Mat& resultMask );
		void updateMasks( vector<Mat> masks );
		void feed (InputArray _img, InputArray mask, Point tl);
		void prepare(Rect dst_roi);
		void blend(InputOutputArray dst, InputOutputArray dst_mask);
		void setSharpness(float val) { sharpness_ = val; }
		
		BlendingProcessor(  int vc, Rect canvasROI, vector<Point> c, vector<Size> s);
		~BlendingProcessor();
};

#endif // _H_BLENDING_PROCESSOR
