#include <header/VideoLoader.h>

void VideoLoader::loadVideos(char* fileName) {
	ifstream inputFile(fileName);

	string videoName;
	while (getline(inputFile, videoName)) {
		VideoCapture* capture = new VideoCapture(videoName);
		if( !capture->isOpened() )
			exitWithMsg(E_FILE_NOT_EXISTS, videoName);
		mVideoList.push_back(capture);
	}
	logMsg(LOG_INFO, "Input videos amount: " + to_string(mVideoList.size()) );
	if (mVideoList.size() <= 1) 
		exitWithMsg(E_TOO_FEW_VIDEOS);
}

void VideoLoader::preloadVideoForDuration(int duration) {
	for (unsigned int v=0; v<mVideoList.size(); v++) {
		vector<Mat> frames;
		Mat frame;
		mVideoList[v]->set(CV_CAP_PROP_POS_FRAMES, 0);
		for (int f=0; f<duration; f++) {
			(*mVideoList[v]) >> frame;
			frames.push_back(frame);
		}
		mPreloadFrames.push_back(frames);
		mVideoList[v]->set(CV_CAP_PROP_POS_FRAMES, 0);
		logMsg(LOG_DEBUG, stringFormat("\tVideo # %d preloaded", v) );
	}
}

int VideoLoader::getVideoListSize() {
	return mVideoList.size();
}

VideoCapture* VideoLoader::getVideo(int idx) {
	return mVideoList[idx];
}

int VideoLoader::getVideoCount() {
	return mVideoList.size();
}

bool VideoLoader::getFrameInSeq(unsigned int fIdx, unsigned int vIdx, Mat& frame) {
	if (vIdx >= mVideoList.size() || fIdx >= mVideoList[vIdx]->get(CV_CAP_PROP_FRAME_COUNT)) {
		logMsg(LOG_ERROR, stringFormat("Unproper frame requesting. [ f: %d, v: %d]", fIdx, vIdx) );
		return false;
	}
	if (fIdx < mPreloadFrames[vIdx].size())
		frame = mPreloadFrames[vIdx][fIdx];
	else {
		mVideoList[vIdx]->set(CV_CAP_PROP_POS_FRAMES, fIdx);
		mVideoList[vIdx]->read(frame);
	}
	return true;
}

void VideoLoader::loadCalibrationFile(char* calFileName) {
	/** 
		[TODO] :
			Implement calibration data structure and load it from file
	**/
}

VideoLoader::VideoLoader(char* inputFileName) {
	if (inputFileName == 0)
		exitWithMsg(E_FILE_NOT_EXISTS);
	loadVideos(inputFileName);
}

VideoLoader::~VideoLoader() {
	for (VideoCapture* cap : mVideoList)
		cap->release();
}