#ifndef _H_VIDEO_LOADER
#define _H_VIDEO_LOADER

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <map>

#include <header/Usage.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

class VideoLoader {
	private:
		vector<VideoCapture*> mVideoList;
		vector< vector<Mat> > mPreloadFrames;
		vector<struct MutualProjectParam> mMutualParams;
		double mFocalLength;
		Size mVideoSize;
		map< string, Mat > mCalibrationMatrix;

		void loadVideos(char* flieName);
		void calcFocalLengthInPixel(double crop, double hfov);
	public:
		double getFocalLength();
		int getVideoListSize();
		VideoCapture* getVideo(int idx);
		bool getFrameInSeq(unsigned int fIdx, unsigned int vIdx, Mat& frame);
		double getVideoFPS();
		int getVideoCount();
		Size getVideoSize();
		map< string, Mat > getCalibrationData();
		vector<struct MutualProjectParam> getPTOData();
		void loadCalibrationFile(char* calFileName);
		void loadPTOFile(char* calFileName);
		void preloadVideoForDuration(int duration);

    	VideoLoader(char* inputFileName);
    	~VideoLoader();
};

#endif // _H_VIDEO_LOADER
