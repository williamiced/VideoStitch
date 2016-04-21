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
#include <atomic> 
#include <thread>
#include <queue>
#include <mutex> 

#include <header/Params.h>
#include <header/Usage.h>
#include <header/ToolBoxParser.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

class VideoLoader {
	private:
		vector<VideoCapture*> mVideoList;
		vector< queue<Mat> > mFrameBuffers;
		vector<struct MutualProjectParam> mMutualParams;
		double mFocalLength;
		Size mVideoSize;
		map< string, Mat > mCalibrationMatrix;
		int mCurrentFirstFrame;
		int mDuration;
		mutex mBufLock;
		int mVideoType;

		// For thread control
		thread mBufferProducerThread;
		atomic<bool> mIsProducerRun;

		bool mIsFinish;

		void loadVideos(char* flieName);
		void calcFocalLengthInPixel(double crop, double hfov);
		bool wakeLoaderUp();
	public:
		int getVideoType();
		double getFocalLength();
		int getVideoListSize();
		VideoCapture* getVideo(int idx);
		bool getFrameInSeq(unsigned int fIdx, unsigned int vIdx, Mat& frame);
		double getVideoFPS();
		int getVideoCount();
		Size getVideoSize();
		vector<Mat> getCalibrationData(string id);
		vector<struct MutualProjectParam> getPTOData();
		void loadCalibrationFile(char* calFileName);
		void loadCalibrationFileFromToolBox(char* calFileName);
		void loadFeatureInfoFromFile(char* fileName, vector<MatchInfo>& matchInfos);
		void loadPTOFile(char* calFileName);
		void preloadVideo();
		bool isFinish();
		bool isCleanup();

    	VideoLoader(char* inputFileName, int duration);
    	~VideoLoader();
};

#endif // _H_VIDEO_LOADER
