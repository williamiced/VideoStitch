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
#include <sys/stat.h>
#include <unistd.h>
#include <header/ExposureProcessor.h>
#include <header/BlendingProcessor.h>
#include <header/VideoLoader.h>
#include <header/Params.h>
#include <header/Usage.h>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/vector.hpp>

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
		vector<MatchInfo> mMatchInfos;
		vector< vector<Mat> > mHs;
		vector<Mat> mFinalBlendingMap;

		bool checkSeriailFileExist(string filename);
		void setupWarpers();
		void defineWindowSize();
		void initialData();
		void interpolateUVcheckupTable();
		void constructUVcheckupTable();
		void refineCheckupTableByFeaturesMatching();
		void drawMatches(Mat& img);
		vector<Vec3b> getPixelsValueByUV(float u, float v, vector<Mat> frames, Mat& mask);
		void tuneToMap(Point2f& p);
		void getUVbyAzimuthal(const float xOffset, const float yOffset, const Point2f center, Point2f& newPnt);
		int rad2Deg(float r);
		float deg2Rad(int d);
		void loadSerialFile();
		void saveSerialFile();

	public:
		MappingProjector(int viewCount, Size viewSize);
		void renderPartialPano(Mat& outImg, vector<Mat> frames, Rect renderArea, Mat renderMask);
		void calcProjectionMatrix();
		Size getOutputVideoSize();
		void setCameraParams(vector<struct MutualProjectParam> params, double focalLength);
		void setCameraParams(vector<Mat> Rs, vector<Mat> Ks);
		void checkFPS();
		void saveMatchInfos(vector<MatchInfo> matchInfos);
};

BOOST_SERIALIZATION_SPLIT_FREE(Mat)
namespace boost {
namespace serialization {

    /*** Mat ***/
    template<class Archive>
    void save(Archive & ar, const Mat& m, const unsigned int version)
    {
      size_t elemSize = m.elemSize(), elemType = m.type();

      ar & m.cols;
      ar & m.rows;
      ar & elemSize;
      ar & elemType; // element type.
      size_t dataSize = m.cols * m.rows * m.elemSize();

      //cout << "Writing matrix data rows, cols, elemSize, type, datasize: (" << m.rows << "," << m.cols << "," << m.elemSize() << "," << m.type() << "," << dataSize << ")" << endl;

      for (size_t dc = 0; dc < dataSize; ++dc) {
          ar & m.data[dc];
      }
    }

    template<class Archive>
    void load(Archive & ar, Mat& m, const unsigned int version)
    {
        int cols, rows;
        size_t elemSize, elemType;

        ar & cols;
        ar & rows;
        ar & elemSize;
        ar & elemType;

        m.create(rows, cols, elemType);
        size_t dataSize = m.cols * m.rows * elemSize;

        //cout << "reading matrix data rows, cols, elemSize, type, datasize: (" << m.rows << "," << m.cols << "," << m.elemSize() << "," << m.type() << "," << dataSize << ")" << endl;

        for (size_t dc = 0; dc < dataSize; ++dc) {
                  ar & m.data[dc];
        }
    }

}
}

#endif // _H_MAPPING_PROJECTOR
