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

void VideoStitcher::calcProjectionMatrix() {
	int seqCount = mVL->getVideoCount();
	for (int v=0; v<seqCount; v++) 
		mProjMat.push_back(Mat::eye(2, 3, CV_32F));
	/** 
		[TODO]
			Use calibration file to get the proejction matrix
	*/
}

void VideoStitcher::projectOnCanvas(GpuMat& canvas, Mat frame, int vIdx) {
	GpuMat newFrame;
	GpuMat oriFrame(frame);
	cv::cuda::warpAffine(oriFrame, newFrame, mProjMat[vIdx], Size(frame.cols, frame.rows));
	
	// [TODO] Currently copyTo is a bottleneck which reduce fps from 40 to 13
	newFrame.copyTo( canvas );
	/** 
		[TODO]
			Paste new frame onto canvas
	*/
}

VideoStitcher::~VideoStitcher() {
	delete mVL;
	delete mLP;
	delete mEP;
	delete mVS;
}

VideoStitcher::VideoStitcher(int argc, char* argv[]) {
	/** 
		[Do preprocess]
			1. Load videos
			2. Load calibration files
			3. Calculate projection matrixs
	*/
	logMsg(LOG_INFO, "[Do preprocess...]");
	mVL = new VideoLoader( getCmdOption(argv, argv + argc, "--input") );
	mVL->loadCalibrationFile( getCmdOption(argv, argv + argc, "--calibration") );
	mVL->preloadVideoForDuration( stoi( getCmdOption(argv, argv + argc, "--duration")) );
	calcProjectionMatrix();
	mLP = new LensProcessor();
	mEP = new ExposureProcessor();
	mVS = new VideoStablizer();
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
	logMsg(LOG_INFO, "[Do real-time process...]");
	double videoFPS = mVL->getVideoFPS();
	Size outputSize = Size(2704, 2028); // [TODO]: not decided yet
	
	VideoWriter* outputVideo = new VideoWriter( getCmdOption(argv, argv + argc, "--output"), CV_FOURCC('D', 'I', 'V', 'X'), videoFPS, outputSize);

	int seqCount = mVL->getVideoCount();
	int duration = stoi( getCmdOption(argv, argv + argc, "--duration") );
	for (int f=0; f<duration; f++) {
		logMsg(LOG_DEBUG, stringFormat("\tProcess frame # %d", f ));
		GpuMat targetCanvas;
		for (int v=0; v<seqCount; v++) {
			Mat frame;
			mVL->getFrameInSeq(f, v, frame);
			mLP->undistort(frame);
			projectOnCanvas(targetCanvas, frame, v);
		}
		mEP->exposureBlending(targetCanvas);
		mVS->stablize(targetCanvas);
		
		Mat canvas;
		targetCanvas.download(canvas);	
		
		(*outputVideo) << canvas;
		
	}
	logMsg(LOG_INFO, "[Done stitching]");
}

int main(int argc, char* argv[]) {
	if ( !checkArguments(argc, argv) )
		exitWithMsg(E_BAD_ARGUMENTS);
	VideoStitcher* vs = new VideoStitcher(argc, argv);
	vs->doRealTimeStitching(argc, argv);

	delete vs;
}