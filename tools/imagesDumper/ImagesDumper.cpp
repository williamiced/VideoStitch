#include "ImagesDumper.h"

ImagesDumper::ImagesDumper(char* fileName, int startFrame, int duration, int step = 1, string outputLoc = "dumpImages/") {
	// Load videos 
	ifstream inputFile(fileName);
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
	int minFrameCount = mVideoList[0]->get(CV_CAP_PROP_FRAME_COUNT);
	for (int v=1; v<mVideoList.size(); v++) 
		minFrameCount = min(minFrameCount, (int) mVideoList[v]->get(CV_CAP_PROP_FRAME_COUNT) );

	for (int v=0; v<mVideoList.size(); v++) {
		for (int i=0; i<mDuration; i += mStep) {
			int f = mStartFrame + i;
			if (f >= minFrameCount)
				continue;
			mVideoList[v]->set(CV_CAP_PROP_POS_FRAMES, f);
			Mat frame;
			bool suc1 = mVideoList[v]->grab();
			bool suc2 = mVideoList[v]->retrieve(frame);
			if (!suc1 || !suc2) {
				cout << "Error when retreiving frame #" << f << " in video #" << v << endl;
				continue;
			}

			string fileName = stringFormat("%d-%d.png", v+1, f);
			string filePath = mOutputLoc + fileName;
			imwrite(filePath, frame);
			cout << "Save " << filePath << endl;
		}
	}
}

int main(int argc, char** argv) {
	if (argc < 3) {
		cout << "Too few arguments. Should have at least 3 arguments which are [input file name], [start frame] and [duration]." << endl;
		return -1;
	}

	ImagesDumper* imagesDumper;
	if (argc == 3)
		imagesDumper = new ImagesDumper(argv[1], stoi(argv[2]), stoi(argv[3]) );
	else 
		imagesDumper = new ImagesDumper(argv[1], stoi(argv[2]), stoi(argv[3]), stoi(argv[4]), string(argv[5]) );
	imagesDumper->startDumpImages();
	cout << "Successfully done images dumping." << endl;
	return 0;
}