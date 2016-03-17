#ifndef _H_IMAGES_DUMPER
#define _H_IMAGES_DUMPER

#include <iostream>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <header/Usage.h>

#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/xfeatures2d.hpp"

using namespace cv;
using namespace std;
using namespace cv::xfeatures2d;

class ImagesDumper {
	private:
		vector<VideoCapture*> mVideoList;
		int mDuration;
		int mStartFrame;
		int mStep;
		string mOutputLoc;
		Mat mPattern;	

	public:
		void startDumpImages();
		bool hasPattern(Mat img);
		ImagesDumper(char* fileName, char* patternName, int startFrame, int duration, int step, string outputLoc);
		~ImagesDumper();
};

#endif // _H_IMAGES_DUMPER