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
		shared_ptr<cv::detail::SphericalWarper> mSphericalWarper;
		vector< Mat > mR;
		
		vector<struct MutualProjectParam> mViewParams;
		vector< vector <Mat> > mProjMap;
		Mat mA; // Also known as K
		Mat mD;
		double mFocalLength;
		int mViewCount;
		Size mViewSize;

		int mDebugView;

		void constructSphereMap();
		Mat calcWeightForEachView(double theta, double phi);
		double getTauAngle(double t1, double p1, double t2, double p2) ;
		void getUVMapping(double t1, double p1, double t2, double p2, double &u, double &v);
		bool isInsideImage( double x, double y );

	public:
		void calcProjectionMatrix(map< string, Mat > calibrationData);
		void projectOnCanvas(Mat& canvas, vector<Mat> frames);

		MappingProjector(int viewCount, Size viewSize, vector<struct MutualProjectParam> params, double focalLength);
		~MappingProjector();
};

#endif // _H_MAPPING_PROJECTOR
