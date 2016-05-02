#ifndef _H_VIDEO_STITCH
#define _H_VIDEO_STITCH

#include <iostream>
#include <algorithm>
#include <ctime> 
#include <queue>
#include <header/VideoLoader.h>
#include <header/LensProcessor.h>
#include <header/VideoStablizer.h>
#include <header/AlignProcessor.h>
#include <header/MappingProjector.h>
#include <header/RealtimeStreamMaker.h>
#include <header/SensorServer.h>
#include <header/SaliencyMapHandler.h>
#include <header/PerformanceAnalyzer.h>
#include <header/Usage.h>

#include "opencv2/core/cuda.hpp"

using namespace cv::cuda;
using namespace std;

class VideoStitcher {
	private:
		shared_ptr<VideoLoader> mVL;
		shared_ptr<LensProcessor> mLP;
		shared_ptr<AlignProcessor> mAP;
		shared_ptr<VideoStablizer> mVS;
		shared_ptr<MappingProjector> mMP;
		shared_ptr<RealtimeStreamMaker> mRSM;
		shared_ptr<SensorServer> mVSS;
		shared_ptr<SaliencyMapHandler> mSMH;
		shared_ptr<PerformanceAnalyzer> mPA;

		int mOH;
		int mOW;

		Size mOutputVideoSize;

		// Config
		bool mIsRealTimeStreaming;
		String mUseSaliencyMapHandler;

	public:
		void doRealTimeStitching(int argc, char* argv[]);
		VideoStitcher(int argc, char* argv[]);
		~VideoStitcher();
};

#endif // _H_VIDEO_STITCH