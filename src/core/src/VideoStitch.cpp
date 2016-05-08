#include <header/VideoStitch.h>

VideoStitcher::~VideoStitcher() {
	if (mIsRealTimeStreaming) {
		logMsg(LOG_INFO, "Wait Server to finish");
		if (mRSM != nullptr)
			mRSM->waitForServerFinish();
		logMsg(LOG_INFO, "Stitcher release");
	}
}

VideoStitcher::VideoStitcher(int argc, char* argv[]) {
	srand(time(NULL));
	if ( !checkArguments(argc, argv) )
		exitWithMsg(E_BAD_ARGUMENTS);

	loadConfig( getCmdOption(argv, argv + argc, "--config") );
	mOH = getIntConfig("OUTPUT_PANO_HEIGHT");
	mOW = getIntConfig("OUTPUT_PANO_WIDTH");
	mIsRealTimeStreaming = (getIntConfig("REAL_TIME_STREAMING") == 1);
	mUseSaliencyMapHandler = getStringConfig("USE_SALIENCY_MAP_HANDLER");
	/** 
		[Do preprocess]
			1. Load videos
			2. Load calibration files
			3. Calculate projection matrixs
	*/
	logMsg(LOG_INFO, "=== Initialize Performance Analyzer ===");
	mPA = shared_ptr<PerformanceAnalyzer>( new PerformanceAnalyzer() );

	logMsg(LOG_INFO, "=== Do preprocess ===");
	mVL = shared_ptr<VideoLoader>( new VideoLoader( getCmdOption(argv, argv + argc, "--input"), stoi( getCmdOption(argv, argv + argc, "--duration")) ) );
	logMsg(LOG_INFO, "=== Data loaded complete ===");

	if (mUseSaliencyMapHandler.compare("FILE") == 0) {
		logMsg(LOG_INFO, "=== Initialize saliency map handler from File ===");
		mSMH = shared_ptr<SaliencyMapHandler>( new SaliencyMapHandler( getCmdOption(argv, argv + argc, "--saliency"), stoi( getCmdOption(argv, argv + argc, "--duration")) ) );
	} else if (mUseSaliencyMapHandler.compare("KLT") == 0) {
		logMsg(LOG_INFO, "=== Initialize saliency map handler from KLT ===");
		mSMH = shared_ptr<SaliencyMapHandler>( new SaliencyMapHandler( ) );
	} else {
		logMsg(LOG_INFO, "=== Run without salienct handler ===");
	}

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

	if (mIsRealTimeStreaming) {
		logMsg(LOG_INFO, "=== Wait for user to connet ===");
		if (getStringConfig("USING_PROTOCAL").compare("UDP") == 0) {
			while ( !mVSS->isSensorWorks() );

			logMsg(LOG_INFO, "=== Initialize Real-time Stream Maker ===");
			mRSM = shared_ptr<RealtimeStreamMaker>( new RealtimeStreamMaker(argc, argv, mVSS->getClientIP()) );
		} else if (getStringConfig("USING_PROTOCAL").compare("TCP") == 0) { // When using TCP, server should be set up before the client connection
			logMsg(LOG_INFO, "=== Initialize Real-time Stream Maker ===");
			mRSM = shared_ptr<RealtimeStreamMaker>( new RealtimeStreamMaker(argc, argv, mVSS->getClientIP()) );

			while ( !mVSS->isSensorWorks() );
		} else { // RTSP
			//while ( !mVSS->isSensorWorks() );

			logMsg(LOG_INFO, "=== Initialize Real-time Stream Maker ===");
			mRSM = shared_ptr<RealtimeStreamMaker>( new RealtimeStreamMaker(argc, argv, mVSS->getClientIP()) );
		}
	}
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

	VideoWriter* outputVideo = new VideoWriter( getCmdOption(argv, argv + argc, "--output"), mVL->getVideoType(), mVL->getVideoFPS(), mMP->getOutputVideoSize() );

	int seqCount = mVL->getVideoCount();
	int duration = stoi( getCmdOption(argv, argv + argc, "--duration") );
	
	Rect renderArea = Rect(0, 0, mOW, mOH);
	Mat  renderMask = Mat(mOH, mOW, CV_8UC1, 1);
	int  renderDiameter = getIntConfig("OUTPUT_PANO_WIDTH");
	Point2f renderCenter = Point2f(0.5f, 0.5f);

	int saliencyMode = 0;
	if (mUseSaliencyMapHandler.compare("FILE") == 0)
		saliencyMode = 1;
	else if (mUseSaliencyMapHandler.compare("KLT") == 0)
		saliencyMode = 2;

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
			if (saliencyMode > 0) 
				mVSS->getFovealInfo(renderDiameter, renderCenter);
			else
				mVSS->getRenderArea(renderArea, renderMask);
		}

		boost::timer::cpu_timer boostTimer;

		if ( saliencyMode > 0 ) {
			mMP->renderSmallSizePano(smallCanvas, frames);
			
			Mat saliencyFrame;
			if ( saliencyMode == 1 ) { // Saliency Map from file
				if ( !mSMH->getSaliencyFrameFromVideo(saliencyFrame) ) 
					exitWithMsg(E_RUNTIME_ERROR, "Error when get saliency frame");
			} else if (saliencyMode == 2) { // Saliency Map from KLT
				if ( !mSMH->calculateSaliencyFromKLT(smallCanvas, saliencyFrame) ) 
					exitWithMsg(E_RUNTIME_ERROR, "Error when get saliency frame");
			}
			//cv::resize(smallCanvas, targetCanvas, Size(mOW, mOH));
			targetCanvas = Mat::zeros(mOH, mOW, CV_8UC3);
			mMP->renderSaliencyArea(targetCanvas, frames, saliencyFrame, renderDiameter, renderCenter);
		} else { // No saliency 
			mMP->renderPartialPano(targetCanvas, frames, renderArea, renderMask);
		}
		
		if (mIsRealTimeStreaming) {
			mRSM->streamOutFrame(targetCanvas);
		} 

		boostTimer.stop();
		if (f != 0)
			mPA->addExecTime(stod(boostTimer.format(3, "%w")));

		//(*outputVideo) << targetCanvas;
		static int frameCounter = 0;
		imwrite(stringFormat("raw/pic_%d.png", frameCounter), targetCanvas);
		frameCounter++;

		if (f != 0)
			mPA->increaseFrame();
	}
	mPA->checkFPS();
	logMsg(LOG_INFO, "=== Done stitching ===");
}
