#ifndef _H_REF_PROJ_GENERATOR
#define _H_REF_PROJ_GENERATOR

#include <header/VideoLoader.h>
#include <header/Params.h>
#include <header/Usage.h>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/nonfree.hpp"

class FeatureMatchResult {
	public:
		int i;
		int j;
		vector< pair<vector<Point2d>, vector<Point2d> > > matches;
		vector< Mat > transformGs;
};

class RefProjGenerator {
	private:
		VideoLoader* mVL;
		vector<FeatureMatchResult*> mFeatureMatchResults;

		void calMutualPosition(int i, int j);

	public:
		void doReferenceProjection();
		RefProjGenerator(VideoLoader* videoLoader);
		~RefProjGenerator();
};

#endif // _H_REF_PROJ_GENERATOR
