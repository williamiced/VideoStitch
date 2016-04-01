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
#include <header/Usage.h>

#include "opencv2/core/cuda.hpp"

using namespace cv::cuda;
using namespace std;

typedef void (*transmitFuncPtr) ( Mat );
typedef void (*renderRegionUpdaterPtr) ( float&, float&, float& );

class VideoStitcher {
	private:
		shared_ptr<VideoLoader> mVL;
		shared_ptr<LensProcessor> mLP;
		shared_ptr<AlignProcessor> mAP;
		shared_ptr<VideoStablizer> mVS;
		shared_ptr<MappingProjector> mMP;
		shared_ptr<RealtimeStreamMaker> mRSM;

		transmitFuncPtr mTransmitFunc;
		renderRegionUpdaterPtr mRenderRegionUpdater;
		Size mOutputVideoSize;
		float mRenderCenterU;
		float mRenderCenterV;
		float mRenderRange;

	public:
		void doRealTimeStitching(int argc, char* argv[]);
		void registerCallbackFunc ( transmitFuncPtr p );
		void registerUpdaterFunc ( renderRegionUpdaterPtr p );
		VideoStitcher(int argc, char* argv[]);
		~VideoStitcher();
};

#endif // _H_VIDEO_STITCH