#ifndef _H_MAPPING_PROJECTOR
#define _H_MAPPING_PROJECTOR

#include <map>
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
		vector< vector<Mat> > mProjMat;
		Mat mA; // Also known as K
		Mat mD;
		Mat mProjMap;
		vector<cv::Point2d> mViewCenter;
		double mFocalLength;
		int mViewCount;
		Size mViewSize;

		void getAngleDiff(double& theta, double& phi, int vIdx);
		Mat calcWeightForEachView(double theta, double phi);
		bool isViewCloseEnough(double theta, double phi, int vIdx);

	public:
		void calcProjectionMatrix(map< string, Mat > calibrationData);
		void projectOnCanvas(Mat& canvas, vector<Mat> frames);

		MappingProjector(int viewCount, Size viewSize);
		~MappingProjector();
};

#endif // _H_MAPPING_PROJECTOR
