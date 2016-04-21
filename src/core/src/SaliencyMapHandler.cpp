#include <header/SaliencyMapHandler.h>

void SaliencyMapHandler::loadSaliencyVideo(char* saliencyFileName) {
	mSaliencyVideo = new VideoCapture(saliencyFileName);
	mW = (int) mSaliencyVideo->get(CV_CAP_PROP_FRAME_WIDTH);
	mH = (int) mSaliencyVideo->get(CV_CAP_PROP_FRAME_HEIGHT);

	mSaliencyReaderThread = thread(&SaliencyMapHandler::preloadSaliencyVideo, this);
}

void SaliencyMapHandler::analyzeInfo(Mat img, Mat& info) {
	int h = mH / SALIENCY_GRID_SIZE;
	int w = mW / SALIENCY_GRID_SIZE;
	info = Mat::zeros(h, w, CV_8UC1);
	
	for (int y=0; y<h; y++) {
		for (int x=0; x<w; x++) {
			int total = 0;
			for (int y0=y*SALIENCY_GRID_SIZE; y0<(y+1)*SALIENCY_GRID_SIZE; y0++) {
				for (int x0=x*SALIENCY_GRID_SIZE; x0<(x+1)*SALIENCY_GRID_SIZE; x0++) {
					if (img.at<Vec3b>(y0, x0)[0] > SALIENCY_THRESH)
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

	while (mSaliencyBuffer.size() < VIDEO_CONTAINER_SIZE && !mIsFinish) {
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

bool SaliencyMapHandler::getSaliencyFrame(Mat& frame) {
	if (isCleanup())
		return false;

	while (mSaliencyBuffer.size() == 0)	 {
		if (!mIsProducerRun)
			wakeLoaderUp();
	}
	
	mBufLock.lock();
	frame = mSaliencyBuffer.front();
	mSaliencyBuffer.pop();
	mBufLock.unlock();
	wakeLoaderUp();

	return true;
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

SaliencyMapHandler::SaliencyMapHandler(char* saliencyFileName, int duration) : mCurrentFirstFrame(0), mIsProducerRun(false), mIsFinish(false), mDuration(duration) {
	loadSaliencyVideo(saliencyFileName);
}

SaliencyMapHandler::~SaliencyMapHandler() {
	mSaliencyVideo->release();
}