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
#include <header/Usage.h>

#include "opencv2/core/cuda.hpp"

using namespace cv::cuda;
using namespace std;

typedef void (*transmitFuncPtr) ( Mat );
typedef void (*renderRegionUpdaterPtr) ( float&, float&, float&, float& );

class VideoStitcher {
	private:
		shared_ptr<VideoLoader> mVL;
		shared_ptr<LensProcessor> mLP;
		shared_ptr<AlignProcessor> mAP;
		shared_ptr<VideoStablizer> mVS;
		shared_ptr<MappingProjector> mMP;

		transmitFuncPtr mTransmitFunc;
		renderRegionUpdaterPtr mRenderRegionUpdater;
		Size mOutputVideoSize;
		float mRenderU1;
		float mRenderU2;
		float mRenderV1;
		float mRenderV2;

	public:
		void doRealTimeStitching(int argc, char* argv[]);
		void registerCallbackFunc ( transmitFuncPtr p );
		void registerUpdaterFunc ( renderRegionUpdaterPtr p );
		VideoStitcher(int argc, char* argv[]);
		~VideoStitcher();
};

#endif // _H_VIDEO_STITCH