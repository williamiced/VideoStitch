#ifndef _H_VIDEO_LOADER
#define _H_VIDEO_LOADER

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <header/Usage.h>

#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

class VideoLoader {
	private:
		vector<VideoCapture*> mVideoList;

		void loadVideos(char* flieName);
	public:

		int getVideoListSize();
		VideoCapture* getVideo(int idx);

    	VideoLoader(char* inputFileName);
    	~VideoLoader();
};

#endif // _H_VIDEO_LOADER
