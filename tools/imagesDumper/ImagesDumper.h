#ifndef _H_IMAGES_DUMPER
#define _H_IMAGES_DUMPER

#include <iostream>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include "../../src/header/Usage.h"

#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

class ImagesDumper {
	private:
		vector<VideoCapture*> mVideoList;
		int mDuration;
		int mStartFrame;
		int mStep;
		string mOutputLoc;

	public:
		void startDumpImages();
		ImagesDumper(char* fileName, int startFrame, int duration, int step, string outputLoc);
		~ImagesDumper();
};

#endif // _H_IMAGES_DUMPER