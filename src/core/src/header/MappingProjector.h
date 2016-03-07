#ifndef _H_MAPPING_PROJECTOR
#define _H_MAPPING_PROJECTOR

#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <set>
#include <map>
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
		vector<Mat> mWarpedImgs;
		vector<Mat> mProjMapX;
		vector<Mat> mProjMapY;
		vector<Mat> mProjMasks;

		void setupWarpers();
		void defineWindowSize();
		void initialData();
		void interpolateUVcheckupTable();
		void constructUVcheckupTable();
		void constructBlendingWeightMap();
		vector<Vec3b> getPixelsValueByUV(float u, float v, vector<Mat> frames, Mat& mask);
		void tuneToMap(Point2f& p);
		void getUVbyAzimuthal(const float xOffset, const float yOffset, const Point2f center, Point2f& newPnt);
		int rad2Deg(float r);
		float deg2Rad(int d);

	public:
		MappingProjector(int viewCount, Size viewSize);
		void renderInterestArea(Mat& outImg, vector<Mat> frames, Point2f center, float renderRange);
		void renderPartialPano(Mat& outImg, vector<Mat> frames, Rect renderArea);
		void calcProjectionMatrix();
		Size getOutputVideoSize();
		void setCameraParams(vector<struct MutualProjectParam> params, double focalLength);
		void setCameraParams(vector<Mat> Rs, vector<Mat> Ks);
		void checkFPS();
};

#endif // _H_MAPPING_PROJECTOR
