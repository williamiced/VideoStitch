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
	/** 
		[Do preprocess]
			1. Load videos
			2. Load calibration files
			3. Calculate projection matrixs
	*/
	logMsg(LOG_INFO, "=== Do preprocess ===");
	mVL = shared_ptr<VideoLoader>( new VideoLoader( getCmdOption(argv, argv + argc, "--input") ) );
	mVL->loadCalibrationFile( getCmdOption(argv, argv + argc, "--calibration") );
	mVL->preloadVideoForDuration( stoi( getCmdOption(argv, argv + argc, "--duration")) );
	
	mLP = shared_ptr<LensProcessor>(new LensProcessor( mVL->getCalibrationData(), mVL->getVideoSize() ));
	mEP = shared_ptr<ExposureProcessor>(new ExposureProcessor());
	mVS = shared_ptr<VideoStablizer>(new VideoStablizer());
	mMP = shared_ptr<MappingProjector>( new MappingProjector(mVL->getVideoCount(), mVL->getVideoSize() ));

	mMP->calcProjectionMatrix( mVL->getCalibrationData() );
	logMsg(LOG_INFO, "Complete projection matrix calculation");
	/*
	Mat image = imread("data/testImg.JPG", CV_LOAD_IMAGE_COLOR);
	mLP->undistort(image);
	imwrite("data/undistort.JPG", image);
	*/
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
	Size outputSize = mVL->getVideoSize(); // [TODO]: not decided yet
	
	VideoWriter* outputVideo = new VideoWriter( getCmdOption(argv, argv + argc, "--output"), CV_FOURCC('D', 'I', 'V', 'X'), videoFPS, Size(OUTPUT_PANO_WIDTH, OUTPUT_PANO_HEIGHT));

	int seqCount = mVL->getVideoCount();
	int duration = stoi( getCmdOption(argv, argv + argc, "--duration") );
	for (int f=0; f<duration; f++) {
		logMsg(LOG_DEBUG, stringFormat("\tProcess frame # %d", f ));
		Mat targetCanvas(OUTPUT_PANO_HEIGHT, OUTPUT_PANO_WIDTH, CV_8UC3);
		vector<Mat> frames;
		for (int v=0; v<seqCount; v++) {
			Mat frame;
			mVL->getFrameInSeq(f, v, frame);
			mLP->undistort(frame);
			frames.push_back(frame);
		}
		mMP->projectOnCanvas(targetCanvas, frames);
		//mEP->exposureBlending(targetCanvas);
		//mVS->stablize(targetCanvas);
		
		//Mat canvas;
		//targetCanvas.download(canvas);	
		cout << targetCanvas.cols << ", " << targetCanvas.rows << endl;
		(*outputVideo) << targetCanvas;
		
	}
	logMsg(LOG_INFO, "=== Done stitching ===");
}

int main(int argc, char* argv[]) {
	if ( !checkArguments(argc, argv) )
		exitWithMsg(E_BAD_ARGUMENTS);
	VideoStitcher* vs = new VideoStitcher(argc, argv);
	vs->doRealTimeStitching(argc, argv);

	delete vs;
}
