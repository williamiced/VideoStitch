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

	mPreloadFrames.resize(mVideoList.size());
	preloadVideo();
}

void VideoLoader::preloadVideo() {
	for (unsigned int v=0; v<mVideoList.size(); v++) {
		mPreloadFrames[v].clear();
		mVideoList[v]->set(CV_CAP_PROP_POS_FRAMES, mCurrentFirstFrame);
		for (int f=0; f<VIDEO_CONTAINER_SIZE; f++) {
			if (f + mCurrentFirstFrame >= mDuration || f + mCurrentFirstFrame >= mVideoList[v]->get(CV_CAP_PROP_FRAME_COUNT) )
				break;
			Mat frame;
			bool suc1 = mVideoList[v]->grab();
			if (!suc1) {
				logMsg(LOG_WARNING, stringFormat("\t\tFrame #%d in video #%d is not read correctly", f + mCurrentFirstFrame, v ) );
			} else {
				bool suc2 = mVideoList[v]->retrieve(frame);
				if (!suc2) 
					logMsg(LOG_WARNING, stringFormat("\t\tFrame #%d in video #%d is not read correctly", f + mCurrentFirstFrame, v ) );
			}
			mPreloadFrames[v].push_back(frame);
		}
		logMsg(LOG_INFO, stringFormat("\tVideo # %d preloaded", v) );
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

vector<Mat> VideoLoader::getCalibrationData(string id) {
	if (id.compare("R") == 0)
		return ToolBoxParser::mR;
	else if (id.compare("K") == 0)
		return ToolBoxParser::mK;
	else if (id.compare("D") == 0)
		return ToolBoxParser::mD;
	else
		return vector<Mat>();
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
	int posInMemory = fIdx - mCurrentFirstFrame;
	if (posInMemory >= static_cast<int>(mPreloadFrames[vIdx].size()) ) {
		mCurrentFirstFrame += VIDEO_CONTAINER_SIZE;
		preloadVideo();
	}
	frame = mPreloadFrames[vIdx][fIdx - mCurrentFirstFrame];
	if (frame.cols == 0)
		return false;

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

void VideoLoader::loadCalibrationFileFromToolBox(char* calFileName) {
	ToolBoxParser::parseToolBoxFile(calFileName);
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

void VideoLoader::loadFeatureInfoFromFile(char* fileName, vector<MatchInfo>& matchInfos) {
	vector<MatchInfo> matcheInfos;

	ifstream inputFile(fileName);

	string str;
	int idx1 = -1;
	int idx2 = -1;
	vector<FeatureMatch> currentFM;
	while (getline(inputFile, str)) {
		if (str.find("#") == 0) {
			if (idx1 >= 0) {
				MatchInfo mi;
				mi.idx1 = idx1;
				mi.idx2 = idx2;
				mi.matches = currentFM;
				matchInfos.push_back(mi);
				currentFM.clear();
			}
			idx1 = str.at(1) - '0';
			idx2 = str.at(4) - '0';
		} else {
			vector<int> tmpVec;
			char * cstr = new char [str.length()+1];
  			std::strcpy (cstr, str.c_str());

  			char * p = std::strtok (cstr,",@");
			while (p!=0) {
				tmpVec.push_back(atoi(p));
			    p = std::strtok(NULL,",@");
			}

			delete[] cstr;

			FeatureMatch fm;
			fm.p1 = Point(tmpVec[0], tmpVec[1]);
			fm.p2 = Point(tmpVec[2], tmpVec[3]);
			currentFM.push_back(fm);
		}
	}
	MatchInfo mi;
	mi.idx1 = idx1;
	mi.idx2 = idx2;
	mi.matches = currentFM;
	matchInfos.push_back(mi);
	currentFM.clear();
}

VideoLoader::VideoLoader(char* inputFileName, int duration):
	mCurrentFirstFrame(0),
	mDuration(duration) {
	if (inputFileName == 0)
		exitWithMsg(E_FILE_NOT_EXISTS);
	loadVideos(inputFileName);
}

VideoLoader::~VideoLoader() {
	for (VideoCapture* cap : mVideoList)
		cap->release();
}