#ifndef _H_EXPOSURE_PROCESSOR
#define _H_EXPOSURE_PROCESSOR

#include <header/VideoLoader.h>
#include <header/Params.h>
#include <header/Usage.h>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/nonfree.hpp"
class ExposureProcessor {
	private:

	public:
		void exposureBlending(Mat& canvas);
		ExposureProcessor();
		~ExposureProcessor();
};

#endif // _H_EXPOSURE_PROCESSOR
