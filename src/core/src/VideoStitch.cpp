#include <header/VideoStitch.h>

char* getCmdOption(char** begin, char** end, const std::string & option) {
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end) 
        return *itr;
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option) {
    return std::find(begin, end, option) != end;
}

bool checkArguments(int argc, char** argv) {
	if ( !cmdOptionExists(argv, argv + argc, "--input") )
		return false;
	return true;
}

VideoStitcher::~VideoStitcher() {
}

VideoStitcher::VideoStitcher(int argc, char* argv[]) {
	mCallback = nullptr;

	if ( !checkArguments(argc, argv) )
		exitWithMsg(E_BAD_ARGUMENTS);
	/** 
		[Do preprocess]
			1. Load videos
			2. Load calibration files
			3. Calculate projection matrixs
	*/
	logMsg(LOG_INFO, "=== Do preprocess ===");
	mVL = shared_ptr<VideoLoader>( new VideoLoader( getCmdOption(argv, argv + argc, "--input") ) );
	mVL->preloadVideoForDuration( stoi( getCmdOption(argv, argv + argc, "--duration")) );
	logMsg(LOG_INFO, "=== Data loaded complete ===");

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

	logMsg(LOG_INFO, "=== Calculate projection matrix for all views ===");
	mOutputVideoSize = mMP->calcProjectionMatrix();
}

void VideoStitcher::doRealTimeStitching(int argc, char* argv[]) {
	/**
		[Real-time process]
			1. Get each image in sequences
			2. Undistort
			3. Use projection matrixs to project to target canvas
			4. Do exposure compensation
			5. Do stablize
			6. Ouput
	*/
	logMsg(LOG_INFO, "=== Do real-time process ===");
	double videoFPS = mVL->getVideoFPS();

	logMsg(LOG_INFO, stringFormat("Canvas size: [%d X %d]", mOutputVideoSize.width, mOutputVideoSize.height) );
	VideoWriter* outputVideo = new VideoWriter( getCmdOption(argv, argv + argc, "--output"), CV_FOURCC('D', 'I', 'V', 'X'), videoFPS, mOutputVideoSize );

	int seqCount = mVL->getVideoCount();
	int duration = stoi( getCmdOption(argv, argv + argc, "--duration") );
	for (int f=0; f<duration; f++) {
		logMsg(LOG_DEBUG, stringFormat("\tProcess frame # %d", f ));
		Mat targetCanvas;
		vector<Mat> frames;
		for (int v=0; v<seqCount; v++) {
			Mat frame;
			mVL->getFrameInSeq(f, v, frame);
#ifdef USE_LENS_UNDISTORT	
			mLP->undistort(frame);
#endif
			frames.push_back(frame);
		}
		mMP->renderInterestArea(targetCanvas, frames, 0, M_PI/2, M_PI/3, M_PI*2/3);
		//mMP->projectOnCanvas(targetCanvas, frames);
		//mVS->stablize(targetCanvas);
		
		//Mat canvas;
		//targetCanvas.download(canvas);
		if (mCallback != nullptr)
			mCallback(targetCanvas);
		cv::resize(targetCanvas, targetCanvas, mOutputVideoSize);

		(*outputVideo) << targetCanvas;
	}
	//mMP->checkFPS();
	logMsg(LOG_INFO, "=== Done stitching ===");
}

void VideoStitcher::registerCallbackFunc ( function_ptr p ) {
	mCallback = p;
}