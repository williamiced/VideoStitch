#ifndef _H_BLENDING_PROCESSOR
#define _H_BLENDING_PROCESSOR

#include <header/Params.h>
#include <header/Usage.h>
#include "opencv2/stitching/detail/blenders.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/core/cuda.hpp"

using namespace cv::cuda;

class BlendingProcessor {
	private:
		cv::Ptr<cv::detail::Blender> mBlender;
		int mViewCount;
		Rect mCanvasROI;
		vector<Point> mCorners;
		vector<Size> mSizes;
		vector<Mat> mDilateMasks;


	public:
		void doBlending( vector<Mat> warpedImg, Mat& result, Mat& resultMask );
		void updateMasks( vector<Mat> masks );
		BlendingProcessor(  int vc, Rect canvasROI, vector<Point> c, vector<Size> s);
		~BlendingProcessor();
};

#endif // _H_BLENDING_PROCESSOR
