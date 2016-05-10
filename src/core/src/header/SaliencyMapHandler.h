#ifndef _H_SALIENCY_MAP_HANDLER
#define _H_SALIENCY_MAP_HANDLER

#include <header/Params.h>
#include <header/Usage.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>
#include <vector>
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "KLT/VideoVolumeAnalyzer.h"

using namespace cv;

class SaliencyMapHandler {
	private:
		queue<Mat> mSaliencyBuffer;
		VideoCapture* mSaliencyVideo;
		thread mSaliencyReaderThread;

		int mCurrentFirstFrame;
		atomic<bool> mIsProducerRun;
		bool mIsFinish;
		int mDuration;
		mutex mBufLock;
		int mW;
		int mH;
		int mGridSize;
		int mGridThresh;
		int mContainerSize;

		/** For KLT */
		unique_ptr<VideoVolumeAnalyzer> mVVA;
		std::list<FeatureTracker*> mFeatureTrackers;
		float mThreshKLT;
		int mFW;
		int mFH;
		Mat mLastInfo;

		// For temporal coherence
		int mTempCohQueueSize;
		float mTempCohSigma;
		vector<Mat> mInfoVec;
		vector<float> mGaussianWeights;
		float* mFeatureCounter;
		float mTemCohFactor1;
		float mTemCohFactor2;

		void analyzeInfo(Mat img, Mat& info);
		bool isCleanup();
		bool wakeLoaderUp();
		void preloadSaliencyVideo();

	public:
		void loadSaliencyVideo(char* saliencyFileName);
		bool getSaliencyFrameFromVideo(Mat& frame);
		void getSaliencyInfoFromTrackers(Mat& info);
		bool calculateSaliencyFromKLT(Mat& frame, Mat& saliencyInfo);
		SaliencyMapHandler(); // For KTL
		SaliencyMapHandler(char* saliencyFileName, int duration); // For file
		~SaliencyMapHandler();
		
};

#endif // _H_SALIENCY_MAP_HANDLER
