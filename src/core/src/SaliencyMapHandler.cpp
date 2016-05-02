#include <header/SaliencyMapHandler.h>

void SaliencyMapHandler::loadSaliencyVideo(char* saliencyFileName) {
	mSaliencyVideo = new VideoCapture(saliencyFileName);
	mW = (int) mSaliencyVideo->get(CV_CAP_PROP_FRAME_WIDTH);
	mH = (int) mSaliencyVideo->get(CV_CAP_PROP_FRAME_HEIGHT);

	mSaliencyReaderThread = thread(&SaliencyMapHandler::preloadSaliencyVideo, this);
}

void SaliencyMapHandler::analyzeInfo(Mat img, Mat& info) {
	int h = mH / mGridSize;
	int w = mW / mGridSize;
	info = Mat::zeros(h, w, CV_8UC1);
	
	for (int y=0; y<h; y++) {
		for (int x=0; x<w; x++) {
			int total = 0;
			for (int y0=y*mGridSize; y0<(y+1)*mGridSize; y0++) {
				for (int x0=x*mGridSize; x0<(x+1)*mGridSize; x0++) {
					if (img.at<Vec3b>(y0, x0)[0] > mGridThresh)
						total++;
					else
						total--;
				}	
			}
			if (total >= 0)
				info.at<uchar>(y, x) = 255;
			else
				info.at<uchar>(y, x) = 0;
		}
	}
}

void SaliencyMapHandler::preloadSaliencyVideo() {
	mIsProducerRun = true;

	while ( (int) mSaliencyBuffer.size() < mContainerSize && !mIsFinish) {
		Mat frame;
		
		if ( !mSaliencyVideo->read(frame) ) {
			logMsg(LOG_WARNING, stringFormat("Saliency Map frame is broken on idx: %d", mCurrentFirstFrame) , 4);
		} else {
			Mat saliencyInfo;
			analyzeInfo(frame, saliencyInfo);
			mBufLock.lock();
			mSaliencyBuffer.push(saliencyInfo);	
			mBufLock.unlock();
			logMsg(LOG_INFO, stringFormat("Read saliency frame #%d", mCurrentFirstFrame), 4);
		}
		
		mCurrentFirstFrame++;
		if (mCurrentFirstFrame >= mDuration)
			mIsFinish = true;
	}
	mIsProducerRun = false;	
}

bool SaliencyMapHandler::getSaliencyFrameFromVideo(Mat& frame) {
	if (isCleanup())
		return false;

	while (mSaliencyBuffer.size() == 0)	 {
		if (!mIsProducerRun)
			wakeLoaderUp();
		if (mIsFinish)
			break;
	}
	if (mSaliencyBuffer.size() == 0)
		return false;
	
	mBufLock.lock();
	frame = mSaliencyBuffer.front();
	mSaliencyBuffer.pop();
	mBufLock.unlock();
	wakeLoaderUp();

	return true;
}

bool SaliencyMapHandler::calculateSaliencyFromKLT(Mat& frame, Mat& saliencyInfo) {
	Mat featureCanvas;
	cv::resize(frame, featureCanvas, Size(mFW, mFH));
	mVVA->process(featureCanvas);

	mFeatureTrackers = mVVA->getActiveTrackers();

	logMsg(LOG_DEBUG, stringFormat("Current is oK, feature num is %d", mFeatureTrackers.size() )) ;

	getSaliencyInfoFromTrackers(saliencyInfo);

	return true;
}

void SaliencyMapHandler::getSaliencyInfoFromTrackers(Mat& info) {
	if (mCurrentFirstFrame % 5 != 0) {
		mCurrentFirstFrame++;
		info = mLastInfo;
		return;
	}

	int h = mFH / mGridSize;
	int w = mFW / mGridSize;

	info = Mat::zeros(h, w, CV_8UC1);

	memset(mFeatureCounter, 0, sizeof(float) * h*w);
	float unit = 1.f / (mFeatureTrackers.size() + 1e-5); // Deal with divide-by-0 exception

	for(FeatureTracker* tracker : mFeatureTrackers) {
		Point p = tracker->getLastPoint();
		int x = p.x / mGridSize;
		int y = p.y / mGridSize;

		mFeatureCounter[y*w+x] += unit;
	}

	for (int y=0; y<h; y++)
		for (int x=0; x<w; x++)
			info.at<uchar>(y, x) = mFeatureCounter[y*w+x] > mThreshKLT ? 255 : 0;

  	/// Apply the dilation operation
	
	int dilation_size = 3;
	Mat kernel = getStructuringElement (MORPH_ELLIPSE, Size( 2*dilation_size + 1, 2*dilation_size+1 ), Point( dilation_size, dilation_size ));
	dilate( info, info, kernel );

	mLastInfo = info;
	
	mCurrentFirstFrame++;
}

bool SaliencyMapHandler::wakeLoaderUp() {
	if (!mIsProducerRun) {
		if (mSaliencyReaderThread.joinable())
			mSaliencyReaderThread.join();
		
		if (mIsFinish)
			return false;
		mSaliencyReaderThread = thread(&SaliencyMapHandler::preloadSaliencyVideo, this);
	}
	return true;
}

bool SaliencyMapHandler::isCleanup() {
	return mIsFinish && (mSaliencyBuffer.size() == 0);
}

SaliencyMapHandler::SaliencyMapHandler(char* saliencyFileName, int duration) : 
	mCurrentFirstFrame(0), 
	mIsProducerRun(false), 
	mIsFinish(false), 
	mDuration(duration),
 	mGridSize(getIntConfig("SALIENCY_GRID_SIZE")),
 	mGridThresh(getIntConfig("EPSILON_F")),
 	mContainerSize(getIntConfig("VIDEO_CONTAINER_SIZE")) {

	loadSaliencyVideo(saliencyFileName);
}

SaliencyMapHandler::SaliencyMapHandler() :
	mCurrentFirstFrame(0),
	mGridSize(getIntConfig("SALIENCY_GRID_SIZE")),
 	mThreshKLT(getFloatConfig("EPSILON_F_KLT")),
 	mFW(getIntConfig("FEATURE_CANVAS_WIDTH")),
 	mFH(getIntConfig("FEATURE_CANVAS_HEIGHT")) {

 	int h = mFH / mGridSize;
	int w = mFW / mGridSize;
	mFeatureCounter = new float[h*w];
	mVVA = unique_ptr<VideoVolumeAnalyzer>( new VideoVolumeAnalyzer() );
}

SaliencyMapHandler::~SaliencyMapHandler() {
	if (mSaliencyVideo != nullptr)
		mSaliencyVideo->release();
	if (mVVA != nullptr)
		mVVA.release();
	if (mFeatureCounter != nullptr)
		delete mFeatureCounter;
}