#ifndef _H_MAPPING_PROJECTOR
#define _H_MAPPING_PROJECTOR

#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <set>
#include <stack>
#include <boost/timer/timer.hpp>
#include <omp.h>
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
		vector< vector<Mat> > mProjMap;
		Rect mCanvasROI;

		void setupWarpers();
		void buildMapsForViews();
		void updateCurrentCanvasROI();
		void defineWindowSize();
		void interpolateUVcheckupTable();
		void constructUVcheckupTable();
		vector<Vec3b> getPixelsValueByUV(float u, float v, vector<Mat> frames);
		int rad2Deg(float r);
		float deg2Rad(int d);

	public:
		MappingProjector(int viewCount, Size viewSize);
		void renderInterestArea(Mat& outImg, vector<Mat> frames, Point2f center, float renderRange);
		void projectOnCanvas(Mat& canvas, vector<Mat> frames);
		void calcProjectionMatrix();
		Size getOutputVideoSize();
		void setCameraParams(vector<struct MutualProjectParam> params, double focalLength);
		void setCameraParams(vector<Mat> Rs, vector<Mat> Ks);
		void checkFPS();
};

#endif // _H_MAPPING_PROJECTOR
