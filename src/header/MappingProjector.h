#ifndef _H_MAPPING_PROJECTOR
#define _H_MAPPING_PROJECTOR

#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <set>
#include <stack>
#include <header/VideoLoader.h>
#include <header/Params.h>
#include <header/Usage.h>
#include "opencv2/core/core.hpp"
#include "opencv2/core/cuda.hpp"
#include "opencv2/cudawarping.hpp"
#include "opencv2/stitching/warpers.hpp"
#include "opencv2/stitching/detail/exposure_compensate.hpp"

using namespace std;
using namespace cv::cuda;

class MappingProjector {
	private:
		/** 
			Projection Matrix:
				Dimenstion: Width * Height * [ViewCount * (weight, X, Y) ]
		*/
		cv::Ptr<cv::detail::ExposureCompensator> mEC;
		double mFocalLength;
		int mViewCount;
		Size mViewSize;

		shared_ptr<cv::detail::SphericalWarper> mSphericalWarper;
		vector< Mat > mR;
		
		vector<struct MutualProjectParam> mViewParams;
		Mat mA; // Also known as K
		Mat mD;
		
		vector<Mat> mUxMaps;
		vector<Mat> mUyMaps;
		vector<Rect> mMapROIs;
		vector< vector<RenderArea> > mRenderAreas;
		vector<Mat> mMapMasks;
		Rect mCanvasROI;
		Mat mAlphaChannel;
		vector<Mat> mViewAlpha;

		set<int> mDebugView;

		void constructSphereMap();
		void calcRotationMatrix();
		void findingMappingAndROI();
		void constructMasks();
		void constructAlphaChannel();
		void mixWithAlphaChannel(Mat& img, int v);
		Mat getZMatrix(double alpha);
		Mat getYMatrix(double beta);
		Mat getXMatrix(double gamma);

	public:
		Size calcProjectionMatrix(map< string, Mat > calibrationData);
		void projectOnCanvas(Mat& canvas, vector<Mat> frames);

		MappingProjector(int viewCount, Size viewSize, vector<struct MutualProjectParam> params, double focalLength);
		~MappingProjector();
};

#endif // _H_MAPPING_PROJECTOR
