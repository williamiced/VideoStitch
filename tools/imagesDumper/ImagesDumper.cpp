#include "ImagesDumper.h"

ImagesDumper::ImagesDumper(char* fileName, char* patternName, int startFrame, int duration, int step = 1, string outputLoc = "dumpImages/") {
	// Load videos 
	ifstream inputFile(fileName);
	mPattern = imread(patternName);

	string videoName;
	while (getline(inputFile, videoName)) {
		VideoCapture* capture = new VideoCapture(videoName);
		if( !capture->isOpened() ) {
			cout << "Video file not exists: " << videoName << endl;
			exit(-2);
		}
		mVideoList.push_back(capture);
	}	

	mStartFrame = startFrame;
	mDuration = duration;
	mStep = step;
	mOutputLoc = outputLoc;
	if (mOutputLoc.back() != '/')
		mOutputLoc += "/";
}

ImagesDumper::~ImagesDumper() {
	for (VideoCapture* cap : mVideoList)
		cap->release();
}

void ImagesDumper::startDumpImages() {
	cout << "[Start dump images...]" << endl;

	int minFrameCount = mVideoList[0]->get(CV_CAP_PROP_FRAME_COUNT);
	for (int v=1; v<mVideoList.size(); v++) 
		minFrameCount = min(minFrameCount, (int) mVideoList[v]->get(CV_CAP_PROP_FRAME_COUNT) );

	for (int i=0; i<mDuration; i += mStep) {
		vector<Mat> frames;
		
		int f = mStartFrame + i;
		if (f >= minFrameCount)
			continue;

		cout << "frame # " << f << endl;
		
		for (int v=0; v<mVideoList.size(); v++) {
			mVideoList[v]->set(CV_CAP_PROP_POS_FRAMES, f);
			Mat frame;
			bool suc1 = mVideoList[v]->grab();
			bool suc2 = mVideoList[v]->retrieve(frame);
			if (!suc1 || !suc2) {
				cout << "Error when retreiving frame #" << f << " in video #" << v << endl;
				continue;
			}

			frames.push_back(frame);
		}

		if (frames.size() != mVideoList.size())
			continue;

		vector<bool> isHasPattern;
		int hasPatternCount = 0;
		for (int v=0; v<mVideoList.size(); v++) {
			if ( hasPattern(frames[v]) ) {
				hasPatternCount++;
				isHasPattern.push_back(true);
			} else {
				isHasPattern.push_back(false);
			}
		}

		if (hasPatternCount >= 2) {
			for (int v=0; v<mVideoList.size(); v++) {
				if (!isHasPattern[v])
					continue;
				string fileName = stringFormat("%d-%d.png", v+1, f);
				string filePath = mOutputLoc + fileName;
				imwrite(filePath, frames[v]);
				cout << "Save " << filePath << endl;
			}
		}
	}
}

bool ImagesDumper::hasPattern(Mat img) {
	//-- Step 1: Detect the keypoints using SURF Detector
	int minHessian = 400;

	// Feature Matching Algorithm
	Ptr<SURF> FMA = SURF::create( minHessian ) ;

	std::vector<KeyPoint> keypoints_1, keypoints_2;
	FMA->detect( img, keypoints_1 );
	FMA->detect( mPattern, keypoints_2 );

	Mat descriptors_1, descriptors_2;
	FMA->compute( img, keypoints_1, descriptors_1 );
	FMA->compute( mPattern, keypoints_2, descriptors_2 );

	FMA.release();

	//-- Step 3: Matching descriptor vectors using FLANN matcher
	FlannBasedMatcher matcher;

	vector< vector< DMatch>  > matches;
	vector< DMatch > good_matches;
	matcher.knnMatch( descriptors_1, descriptors_2, matches, 2);

	float NNDRRATIO = 0.5;

	for( unsigned int i = 0 ; i < matches.size(); i++){
        if (matches[i].size() < 2)
                continue;
        const DMatch &m1 = matches[i][0];
        const DMatch &m2 = matches[i][1];
        if (m1.distance <= NNDRRATIO * m2.distance)
			good_matches.push_back(m1);
	}

/*
	if (good_matches.size() > 0) {
		Mat img_matches;
		  drawMatches( img, keypoints_1, mPattern, keypoints_2,
		               good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
		               vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

		  //-- Show detected matches
		  imshow( "Good Matches", img_matches );

		  waitKey(0);
	}
*/
	
	return good_matches.size() > 0;
}

int main(int argc, char** argv) {
	if (argc < 4) {
		cout << "Too few arguments. Should have at least 4 arguments which are [input file name], [pattern file name], [start frame] and [duration]." << endl;
		return -1;
	}

	ImagesDumper* imagesDumper;
	if (argc == 4)
		imagesDumper = new ImagesDumper(argv[1], argv[2], stoi(argv[3]), stoi(argv[4]) );
	else 
		imagesDumper = new ImagesDumper(argv[1], argv[2], stoi(argv[3]), stoi(argv[4]), stoi(argv[5]), string(argv[6]) );
	imagesDumper->startDumpImages();
	cout << "Successfully done images dumping." << endl;
	return 0;
}
