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
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"

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

		void analyzeInfo(Mat img, Mat& info);
		bool isCleanup();
		bool wakeLoaderUp();
		void preloadSaliencyVideo();

	public:
		void loadSaliencyVideo(char* saliencyFileName);
		bool getSaliencyFrame(Mat& frame);
		SaliencyMapHandler(char* saliencyFileName, int duration);
		~SaliencyMapHandler();
		
};

#endif // _H_SALIENCY_MAP_HANDLER
