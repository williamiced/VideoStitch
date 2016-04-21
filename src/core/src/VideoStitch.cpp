#include <header/VideoStitch.h>

VideoStitcher::~VideoStitcher() {
#ifdef REAL_TIME_STREAMING
	logMsg(LOG_INFO, "Wait Server to finish");
	if (mRSM != nullptr)
		mRSM->waitForServerFinish();
	logMsg(LOG_INFO, "Stitcher release");
#endif
}

VideoStitcher::VideoStitcher(int argc, char* argv[]) {

	srand(time(NULL));
	if ( !checkArguments(argc, argv) )
		exitWithMsg(E_BAD_ARGUMENTS);
	/** 
		[Do preprocess]
			1. Load videos
			2. Load calibration files
			3. Calculate projection matrixs
	*/
	logMsg(LOG_INFO, "=== Do preprocess ===");
	mVL = shared_ptr<VideoLoader>( new VideoLoader( getCmdOption(argv, argv + argc, "--input"), stoi( getCmdOption(argv, argv + argc, "--duration")) ) );
	logMsg(LOG_INFO, "=== Data loaded complete ===");

#ifdef USE_SALIENCY_MAP_HANDLER
	logMsg(LOG_INFO, "=== Initialize saliency map handler ===");
	mSMH = shared_ptr<SaliencyMapHandler>( new SaliencyMapHandler( getCmdOption(argv, argv + argc, "--saliency"), stoi( getCmdOption(argv, argv + argc, "--duration")) ) );
	logMsg(LOG_INFO, "=== Data loaded complete ===");
#endif

	logMsg(LOG_INFO, "=== Initialize Sensor Server ===");
	mVSS = shared_ptr<SensorServer>( new SensorServer() );
	logMsg(LOG_INFO, "=== Sensor Server is constructed ===");

	logMsg(LOG_INFO, "=== Initialize Video Stablizer ===");
	mVS = shared_ptr<VideoStablizer>(new VideoStablizer());

	logMsg(LOG_INFO, "=== Initialize Mapping Projector ===");
	mMP = shared_ptr<MappingProjector>( new MappingProjector(mVL->getVideoCount(), mVL->getVideoSize()));
	
#ifdef USE_EXTERNAL_CALIBRATION_FILE
	logMsg(LOG_INFO, "=== Load camera parameters from external file ===");
	mVL->loadCalibrationFileFromToolBox( getCmdOption(argv, argv + argc, "--calibration") );
	mMP->setCameraParams ( mVL->getCalibrationData("R"), mVL->getCalibrationData("K") );
#else
	logMsg(LOG_INFO, "=== Load camera parameters from pto file ===");
	mVL->loadPTOFile( getCmdOption(argv, argv + argc, "--pto") );
	mMP->setCameraParams ( mVL->getPTOData(), mVL->getFocalLength() );
#endif

#ifdef USE_LENS_UNDISTORT	
	logMsg(LOG_INFO, "=== Initialize Lens Processor ===");
	mLP = shared_ptr<LensProcessor>(new LensProcessor( mVL->getCalibrationData("K"), mVL->getCalibrationData("D"), mVL->getVideoSize(), mVL->getFocalLength()) );
#endif

#ifdef USE_HOMOGRAPHY_WARP
	logMsg(LOG_INFO, "=== Load Feature Matching Infos From File ===");
	vector<MatchInfo> matchInfos;
	mVL->loadFeatureInfoFromFile( getCmdOption(argv, argv + argc, "--featureInfo"), matchInfos);
	mMP->saveMatchInfos( matchInfos );
#endif

	logMsg(LOG_INFO, "=== Calculate projection matrix for all views ===");
	mMP->calcProjectionMatrix();

	#ifdef REAL_TIME_STREAMING
		logMsg(LOG_INFO, "=== Wait for user to connet ===");
#ifdef USING_UDP
		while ( !mVSS->isSensorWorks() );

		logMsg(LOG_INFO, "=== Initialize Real-time Stream Maker ===");
		mRSM = shared_ptr<RealtimeStreamMaker>( new RealtimeStreamMaker(argc, argv, mVSS->getClientIP()) );
#else // When using TCP, server should be set up before the client connection
		logMsg(LOG_INFO, "=== Initialize Real-time Stream Maker ===");
		mRSM = shared_ptr<RealtimeStreamMaker>( new RealtimeStreamMaker(argc, argv, mVSS->getClientIP()) );

		while ( !mVSS->isSensorWorks() );
#endif
	#endif
}

void VideoStitcher::doRealTimeStitching(int argc, char* argv[]) {
	/**
		[Real-time process]
			1. Get each image in sequences
			2. Undistort
			3. Use projection matrixs to project to target canvas
			4. Do exposure compensation
			5. Do blending
			6. Do stablize
			7. Ouput
	*/
	logMsg(LOG_INFO, "=== Do real-time process ===");

//#ifndef REAL_TIME_STREAMING
	//VideoWriter* outputVideo = new VideoWriter( getCmdOption(argv, argv + argc, "--output"), CV_FOURCC('D', 'I', 'V', 'X'), mVL->getVideoFPS(), mMP->getOutputVideoSize() );
	VideoWriter* outputVideo = new VideoWriter( getCmdOption(argv, argv + argc, "--output"), mVL->getVideoType(), mVL->getVideoFPS(), mMP->getOutputVideoSize() );
//#endif

	int seqCount = mVL->getVideoCount();
	int duration = stoi( getCmdOption(argv, argv + argc, "--duration") );
	
	Rect renderArea = Rect(0, 0, OUTPUT_PANO_WIDTH, OUTPUT_PANO_HEIGHT);
	Mat  renderMask = Mat(OUTPUT_PANO_HEIGHT, OUTPUT_PANO_WIDTH, CV_8UC1, 1);

	for (int f=0; f<duration; f++) {
		if (mVL->isCleanup())
			break;
		bool isHealthyFrame = true;

		logMsg(LOG_INFO, stringFormat("\tProcess frame # %d", f ));
		Mat targetCanvas;
		Mat smallCanvas;
		vector<Mat> frames;
		for (int v=0; v<seqCount; v++) {
			Mat frame;
			if ( !mVL->getFrameInSeq(f, v, frame) ) {
				logMsg(LOG_WARNING, stringFormat("\t=== Frames %d cannot be reached ===", f));
				isHealthyFrame = false;
				break;
			}
#ifdef USE_LENS_UNDISTORT	
			mLP->undistort(frame);
#endif
			frames.push_back(frame);
		}
		if (!isHealthyFrame)
			continue;
		
		if (mVSS->isSensorWorks()) {
			mVSS->getRenderArea(renderArea, renderMask);
		}

#ifndef USE_SALIENCY_MAP_HANDLER
		mMP->renderPartialPano(targetCanvas, frames, renderArea, renderMask);
#else
		mMP->renderSmallSizePano(smallCanvas, frames);
		cv::resize(smallCanvas, targetCanvas, Size(OUTPUT_PANO_WIDTH, OUTPUT_PANO_HEIGHT));
		//targetCanvas = Mat::zeros(OUTPUT_PANO_HEIGHT, OUTPUT_PANO_WIDTH, CV_8UC3);

		Mat saliencyFrame;
		if ( !mSMH->getSaliencyFrame(saliencyFrame) ) 
			exitWithMsg(E_RUNTIME_ERROR, "Error when get saliency frame");
		mMP->renderSaliencyArea(targetCanvas, frames, saliencyFrame);
		//imwrite(stringFormat("tmp2/pano_%d.png", f), targetCanvas);	
#endif
		//imwrite("tmp.png", smallCanvas);
		//mVS->stablize(targetCanvas);
		
#ifdef REAL_TIME_STREAMING
		mRSM->streamOutFrame(targetCanvas);
#ifdef USE_SALIENCY_MAP_HANDLER		
		mRSM->streamOutFrame_small(smallCanvas);
		//(*outputVideo) << targetCanvas;
#endif
#else
		(*outputVideo) << targetCanvas;
#endif
		mMP->increaseFrame();
	}
	mMP->checkFPS();
	logMsg(LOG_INFO, "=== Done stitching ===");
}
