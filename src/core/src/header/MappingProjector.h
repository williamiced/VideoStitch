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
		unsigned int mLastFrameIdx;
		char* mLastFrameTime;

		shared_ptr<ExposureProcessor> mEP;
		shared_ptr<BlendingProcessor> mBP;
		cv::Ptr<cv::detail::Blender> mBlender;
		double mFocalLength;
		int mViewCount;
		Size mViewSize;

		vector< shared_ptr<PROJECT_METHOD> > mSphericalWarpers;
		vector< Mat > mR;
		vector< Mat > mK;
		vector< Mat > mD;
		
		vector<struct MutualProjectParam> mViewParams;
		
		vector<Mat> mUxMaps;
		vector<Mat> mUyMaps;
		vector<Rect> mMapROIs;
		vector<Point> mCorners;
		vector< vector<RenderArea> > mRenderAreas;
		vector<Mat> mMapMasks;
		Rect mCanvasROI;
		Mat mAlphaChannel;
		vector<Mat> mViewAlpha;

		void constructSphereMap();
		void calcRotationMatrix();
		void calcIntrinsicMatrix();
		void findingMappingAndROI();
		void constructMasks();
		void constructAlphaChannel();
		void mixWithAlphaChannel(Mat& img, int v);
		void updateCurrentCanvasROI();
		Mat getZMatrix(double alpha);
		Mat getYMatrix(double beta);
		Mat getXMatrix(double gamma);
		void recalcRotationMatrix();

	public:
		void checkFPS();
		void setCameraParams( vector<Mat> Rs, vector<Mat> Ks, vector<Mat> Ds );
		Size calcProjectionMatrix();
		void projectOnCanvas(Mat& canvas, vector<Mat> frames);

		MappingProjector(int viewCount, Size viewSize, vector<struct MutualProjectParam> params, double focalLength);
		~MappingProjector();
};

#endif // _H_MAPPING_PROJECTOR
