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

VideoStitcher::VideoStitcher(int argc, char* argv[]): 
	mTransmitFunc(nullptr),
	mRenderRegionUpdater(nullptr),
	mRenderCenterU(-3.f),
	mRenderCenterV(2.f),
	mRenderRange(2.f) {

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
	mMP->calcProjectionMatrix();
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

	VideoWriter* outputVideo = new VideoWriter( getCmdOption(argv, argv + argc, "--output"), CV_FOURCC('D', 'I', 'V', 'X'), videoFPS, mMP->getOutputVideoSize() );

	int seqCount = mVL->getVideoCount();
	int duration = stoi( getCmdOption(argv, argv + argc, "--duration") );
	for (int f=0; f<duration; f++) {
		bool isHealthyFrame = true;

		logMsg(LOG_INFO, stringFormat("\tProcess frame # %d", f ));
		Mat targetCanvas;
		vector<Mat> frames;
		for (int v=0; v<seqCount; v++) {
			Mat frame;
			if ( !mVL->getFrameInSeq(f, v, frame) ) {
				logMsg(LOG_WARNING, stringFormat("\t=== Frames %d contain broken frame ===", f));
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
		
		if (mRenderRegionUpdater != nullptr)
			mRenderRegionUpdater(mRenderCenterU, mRenderCenterV, mRenderRange);
#ifdef OUTPUT_PANO
		mMP->projectOnCanvas(targetCanvas, frames);
#else
		mMP->renderInterestArea(targetCanvas, frames, Point2f(mRenderCenterU, mRenderCenterV), mRenderRange);
#endif
		
		
		//mVS->stablize(targetCanvas);
		
		//Mat canvas;
		//targetCanvas.download(canvas);
		if (mTransmitFunc != nullptr)
			mTransmitFunc(targetCanvas);

		(*outputVideo) << targetCanvas;
	}
	mMP->checkFPS();
	logMsg(LOG_INFO, "=== Done stitching ===");
}

void VideoStitcher::registerCallbackFunc ( transmitFuncPtr p ) {
	mTransmitFunc = p;
}

void VideoStitcher::registerUpdaterFunc ( renderRegionUpdaterPtr p ) {
	mRenderRegionUpdater = p;
}