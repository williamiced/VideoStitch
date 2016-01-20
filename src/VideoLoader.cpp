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

	int w = (int) mVideoList[0]->get(CV_CAP_PROP_FRAME_WIDTH);
	int h = (int) mVideoList[0]->get(CV_CAP_PROP_FRAME_HEIGHT);
	mVideoSize = Size(w, h);
}

void VideoLoader::preloadVideoForDuration(int duration) {
	for (unsigned int v=0; v<mVideoList.size(); v++) {
		vector<Mat> frames;
		mVideoList[v]->set(CV_CAP_PROP_POS_FRAMES, 0);
		for (int f=0; f<duration; f++) {
			Mat frame;
			bool suc1 = mVideoList[v]->grab();
			bool suc2 = mVideoList[v]->retrieve(frame);
			frames.push_back(frame);
			if (!suc1 || !suc2)
				logMsg(LOG_WARNING, stringFormat("\t\tFrame #%d in video #%d is not read correctly", f, v) );
		}
		mPreloadFrames.push_back(frames);
		mVideoList[v]->set(CV_CAP_PROP_POS_FRAMES, 0);
		logMsg(LOG_DEBUG, stringFormat("\tVideo # %d preloaded", v) );
	}
}

double VideoLoader::getVideoFPS() {
	return mVideoList[0]->get(CV_CAP_PROP_FPS);
}

int VideoLoader::getVideoListSize() {
	return mVideoList.size();
}

Size VideoLoader::getVideoSize() {
	return mVideoSize;
}

map<string, Mat> VideoLoader::getCalibrationData() {
	return mCalibrationMatrix;
}

vector<struct MutualProjectParam> VideoLoader::getPTOData() {
	return mMutualParams;
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
	FileStorage fs(calFileName, FileStorage::READ);
	Mat A, D;
	fs["cameraMatA"] >> A;
	fs["distCoeffs"] >> D;

	A.convertTo(mCalibrationMatrix["cameraMatA"], CV_32F);
	D.convertTo(mCalibrationMatrix["distCoeffs"], CV_32F);
}

void VideoLoader::loadPTOFile(char* ptoFileName) {
	ifstream ptofile(ptoFileName);
	string line;
	double cropFactor = 1.0f;
	double hfov = 0.0f;

	while ( getline(ptofile, line) ) {
		if ( line.find("#") == 0) { // Comment
			if (line.find("cropFactor=") != string::npos ) 
				cropFactor = stod( line.substr(line.find("cropFactor=") + strlen("cropFactor=")) );
			continue;
		}
		if ( line.find("i") != 0) // We are not care about
			continue;
		
		vector<string> tokens;
		boost::split( tokens, line, boost::is_any_of(" ") );

		struct MutualProjectParam projectParams;
		for (int i=0; i<(int)tokens.size(); i++) {
			if ( tokens[i].find("v") == 0 && tokens[i].find("v=") == string::npos) 
				hfov = stod( tokens[i].substr(1) ) ;
			else if ( tokens[i].find("r") == 0 ) 
				projectParams.r = stod( tokens[i].substr(1) ) * M_PI / 180.f;
			else if ( tokens[i].find("p") == 0 )
				projectParams.p = stod( tokens[i].substr(1) ) * M_PI / 180.f;
			else if ( tokens[i].find("y") == 0 )
				projectParams.y = stod( tokens[i].substr(1) ) * M_PI / 180.f;
			// Need to make clear what is TrX / TrY / TrZ
			else if ( tokens[i].find("TrX") == 0 )
				projectParams.TrX = stod( tokens[i].substr(3) );
			else if ( tokens[i].find("TrY") == 0 )
				projectParams.TrY = stod( tokens[i].substr(3) );
			else if ( tokens[i].find("TrZ") == 0 )
				projectParams.TrZ = stod( tokens[i].substr(3) );
		}
		mMutualParams.push_back(projectParams);
	}

	if (hfov > 0.f) 
		calcFocalLengthInPixel(cropFactor, hfov);
}

double VideoLoader::getFocalLength() {
	return mFocalLength;
}

void VideoLoader::calcFocalLengthInPixel(double crop, double hfov) {
	mFocalLength = 0.f;
	// calculate diagonal of film
    double d = sqrt(36.0f * 36.0f + 24.0f * 24.0f) / crop;
    double r = (double)mVideoSize.width / mVideoSize.height;
    
    // calculate the sensor width and height that fit the ratio
    // the ratio is determined by the size of our image.
    double sensorSizeX;
    sensorSizeX = d / sqrt(1 + 1/(r*r));
    
    // Assume the input is RECTILINEAR
    mFocalLength = (sensorSizeX / 2.0f) / tan( hfov / 180.0f * M_PI / 2);
    // If it is equirectangular
    // mFocalLength = (sensorSizeX / (hfov / 180.f * M_PI));

    mFocalLength = (mFocalLength / sensorSizeX) * mVideoSize.width;
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