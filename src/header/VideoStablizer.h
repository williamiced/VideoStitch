#ifndef _H_VIDEO_STABLIZER
#define _H_VIDEO_STABLIZER

#include <header/VideoLoader.h>
#include <header/Params.h>
#include <header/Usage.h>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/nonfree.hpp"

class VideoStablizer {
	private:

	public:
		void stablize(Mat& canvas);
		VideoStablizer();
		~VideoStablizer();
};

#endif // _H_VIDEO_STABLIZER
