#ifndef _H_MAPPING_PROJECTOR
#define _H_MAPPING_PROJECTOR

#include <iostream>
#include <fstream>
#include <set>
#include <stack>
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
		/** 
			Projection Matrix:
				Dimenstion: Width * Height * [ViewCount * (weight, X, Y) ]
		*/
		double mFocalLength;
		int mViewCount;
		Size mViewSize;

		shared_ptr<cv::detail::SphericalWarper> mSphericalWarper;
		vector< Mat > mR;
		
		vector<struct MutualProjectParam> mViewParams;
		Mat mA; // Also known as K
		Mat mD;
		
		vector<UMat> mUxMaps;
		vector<UMat> mUyMaps;
		vector<Rect> mMapROIs;
		vector<Mat> mMapMasks;
		Rect mCanvasROI;

		set<int> mDebugView;

		void constructSphereMap();
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
