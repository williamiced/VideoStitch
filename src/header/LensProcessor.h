#ifndef _H_LENS_PROCESSOR
#define _H_LENS_PROCESSOR

#include <header/VideoLoader.h>
#include <header/Params.h>
#include <header/Usage.h>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

class LensProcessor {
	private:

	public:
		void undistort(Mat& frame);
		LensProcessor();
		~LensProcessor();
};

#endif // _H_LENS_PROCESSOR
