#ifndef _H_MAPPING_PROJECTOR
#define _H_MAPPING_PROJECTOR

#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <set>
#include <stack>
#include <boost/timer/timer.hpp>
#include <header/ExposureProcessor.h>
#include <header/BlendingProcessor.h>
#include <header/VideoLoader.h>
#include <header/Params.h>
#include <header/Usage.h>
#include "opencv2/core/core.hpp"
#include "opencv2/core/cuda.hpp"
#include "opencv2/cudawarping.hpp"
#include "opencv2/stitching/warpers.hpp"

using namespace std;
using namespace cv::cuda;

class MappingProjector {
	private:
		vector<double> mExecTimes;
		unsigned int mFrameProcessed;

		shared_ptr<ExposureProcessor> mEP;
		shared_ptr<BlendingProcessor> mBP;
		int mViewCount;
		Size mViewSize;
		Size mOutputWindowSize;

		vector< shared_ptr<PROJECT_METHOD> > mSphericalWarpers;
		vector< Mat > mR;
		vector< Mat > mK;
		vector<Mat> mUxMaps;
		vector<Mat> mUyMaps;
		vector<Rect> mMapROIs;
		Rect mCanvasROI;

		void setupWarpers();
		void buildMapsForViews();
		void updateCurrentCanvasROI();
		Vec3b getInversePixel(int y, int x, vector<Mat> frames);
		void defineWindowSize();

	public:
		MappingProjector(int viewCount, Size viewSize);
		void renderInterestArea(Mat& outImg, vector<Mat> frames, float u1, float u2, float v1, float v2);
		void calcProjectionMatrix();
		Size getOutputImageSize();
		void setCameraParams(vector<struct MutualProjectParam> params, double focalLength);
		void setCameraParams(vector<Mat> Rs, vector<Mat> Ks);
};

#endif // _H_MAPPING_PROJECTOR
