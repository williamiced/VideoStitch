#ifndef _H_VIDEO_STITCH
#define _H_VIDEO_STITCH

#include <iostream>
#include <algorithm>
#include <header/VideoLoader.h>
#include <header/Usage.h>

using namespace std;

class VideoStitcher {
	private:
		VideoLoader* mVL;

	public:
		VideoStitcher(int argc, char* argv[]);
		~VideoStitcher();
};

#endif // _H_VIDEO_STITCH