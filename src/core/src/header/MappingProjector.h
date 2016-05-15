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

#include <Cuda/Accelerator.h>

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
		shared_ptr<ExposureProcessor> mEP;
		shared_ptr<BlendingProcessor> mBP;

		int mDW, mDH;
		int mOW, mOH;
		bool mUseSerializableResult;
		int mViewCount;
		cv::Size mViewSize;
		cv::Size mOutputWindowSize;
		bool mUseGPU;
		bool mTurnBlendOn;

		vector< shared_ptr<PROJECT_METHOD> > mSphericalWarpers;
		vector< cv::Mat > mR;
		vector< cv::Mat > mK;
		vector<cv::Mat> mWarpedImgs;
		vector<cv::Mat> mProjMapX;
		vector<cv::Mat> mProjMapY;
		vector<cv::Mat> mProjMasks;
		vector<MatchInfo> mMatchInfos;
		vector< vector<cv::Mat> > mHs;
		vector<cv::Mat> mFinalBlendingMap;

		void getBlendSalinecy(cv::Mat& saliencyInfo, cv::Mat& blendSaliencyInfo, int renderDiameter, cv::Point center, int w, int h, int gridSize);
		bool checkSeriailFileExist(string filename);
		void setupWarpers();
		void defineWindowSize();
		void initialData();
		void interpolateUVcheckupTable();
		void constructUVcheckupTable();
		void refineCheckupTableByFeaturesMatching();
		void drawMatches(cv::Mat& img);
		vector<cv::Vec3b> getPixelsValueByUV(float u, float v, vector<cv::Mat> frames, cv::Mat& mask);
		void tuneToMap(cv::Point2f& p);
		void getUVbyAzimuthal(const float xOffset, const float yOffset, const cv::Point2f center, cv::Point2f& newPnt);
		float distOf(cv::Point p1, cv::Point p2);
		float smoothstep(float edge0, float edge1, float x);
		int rad2Deg(float r);
		float deg2Rad(int d);
		void loadSerialFile();
		void saveSerialFile();

	public:
		MappingProjector(int viewCount, cv::Size viewSize);
		void genExpoBlendingMap(vector<cv::Mat> frames);
		void renderPartialPano(cv::Mat& outImg, vector<cv::Mat> frames, cv::Rect renderArea, cv::Mat renderMask);
		void renderSaliencyArea(cv::Mat& smallImg, cv::Mat& outImg, vector<cv::Mat> frames, cv::Mat saliencyFrame, int renderDiameter, cv::Point2f renderCenter);
		void renderSmallSizePano(cv::Mat& outImg, vector<cv::Mat> frames);
		bool isInDiameter(cv::Point c, cv::Point p, int w, int h, int gridSize, int dSize);
		void calcProjectionMatrix();
		cv::Size getOutputVideoSize();
		void setCameraParams(vector<struct MutualProjectParam> params, double focalLength);
		void setCameraParams(vector<cv::Mat> Rs, vector<cv::Mat> Ks);
		void saveMatchInfos(vector<MatchInfo> matchInfos);
};

BOOST_SERIALIZATION_SPLIT_FREE(cv::Mat)
namespace boost {
namespace serialization {

    /*** Mat ***/
    template<class Archive>
    void save(Archive & ar, const cv::Mat& m, const unsigned int version)
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
    void load(Archive & ar, cv::Mat& m, const unsigned int version)
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
