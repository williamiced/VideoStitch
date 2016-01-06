#ifndef _H_MAPPING_PROJECTOR
#define _H_MAPPING_PROJECTOR

#include <map>
#include <header/VideoLoader.h>
#include <header/Params.h>
#include <header/Usage.h>
#include "opencv2/core/core.hpp"
#include "opencv2/core/cuda.hpp"
#include "opencv2/cudawarping.hpp"
#include "opencv2/stitching/warpers.hpp"

using namespace std;
using namespace cv::cuda;

class MappingProjector {
	private:
		vector<Mat> mProjMat;
		Mat mA; // Also known as K
		Mat mRT;

	public:
		void calcProjectionMatrix(map<string, Mat> calibrationData);
		void projectOnCanvas(GpuMat& canvas, Mat frame, int vIdx);

		MappingProjector();
		~MappingProjector();
};

#endif // _H_MAPPING_PROJECTOR
