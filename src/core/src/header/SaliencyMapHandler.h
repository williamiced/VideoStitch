#ifndef _H_SALIENCY_MAP_HANDLER
#define _H_SALIENCY_MAP_HANDLER

#include <header/Params.h>
#include <header/Usage.h>
#include <Array.h>
#include <fftw++.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>
#include <vector>
#include <omp.h>
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "KLT/VideoVolumeAnalyzer.h"

#define PQFT_THRESH 75
#define PQFT_SCALE 2000

class SaliencyMapHandler {
	private:
		queue<cv::Mat> mSaliencyBuffer;
		cv::VideoCapture* mSaliencyVideo;
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
		cv::Mat mLastInfo;

		/** For PQFT **/
		int* mC_r;
		int* mC_g;
		int* mC_b;
		int* mC_R;
		int* mC_G;
		int* mC_B;
		int* mC_Y;
		int* mC_RG;
		int* mC_BY;
		int* mC_I;
		int* mC_I_past;
		int* mC_M;
		vector<int*> mI_record;
		int mCurrentI;

		Array::array2<Complex>* mf1;
		Array::array2<Complex>* mf2;
		fftwpp::fft2d* mForward1;
		fftwpp::fft2d* mBackward1;
		fftwpp::fft2d* mForward2;
		fftwpp::fft2d* mBackward2;

		// For temporal coherence
		int mTempCohQueueSize;
		float mTempCohSigma;
		vector<cv::Mat> mInfoVec;
		vector<float> mGaussianWeights;
		float* mFeatureCounter;
		float mTemCohFactor1;
		float mTemCohFactor2;
		bool mIsKLT;

		void initPQFT();
		void analyzeInfo(cv::Mat img, cv::Mat& info);
		bool isCleanup();
		bool wakeLoaderUp();
		void preloadSaliencyVideo();

	public:
		void loadSaliencyVideo(char* saliencyFileName);
		bool getSaliencyFrameFromVideo(cv::Mat& frame);
		void getSaliencyInfoFromTrackers(cv::Mat& info);
		bool calculateSaliencyFromKLT(cv::Mat& frame, cv::Mat& saliencyInfo);
		bool calculateSaliencyFromPQFT(cv::Mat& frame, cv::Mat& saliencyInfo);
		SaliencyMapHandler(bool isKLT); // For KTL and PQFT
		SaliencyMapHandler(char* saliencyFileName, int duration); // For file
		~SaliencyMapHandler();
		
};

#endif // _H_SALIENCY_MAP_HANDLER
