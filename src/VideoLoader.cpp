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

int VideoLoader::getVideoListSize() {
	return mVideoList.size();
}

VideoCapture* VideoLoader::getVideo(int idx) {
	return mVideoList[idx];
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