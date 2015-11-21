#include <header/RefProjGenerator.h>

void RefProjGenerator::calMutualPosition(int idxI, int idxJ) {
	VideoCapture* videoI = mVL->getVideo(idxI);
	VideoCapture* videoJ = mVL->getVideo(idxJ);
	int frameCountI = videoI->get(CV_CAP_PROP_FRAME_COUNT);
	int frameCountJ = videoJ->get(CV_CAP_PROP_FRAME_COUNT);
	int calCountLimit = min(frameCountI, frameCountJ);

	Mat frameI, frameJ;
	vector<KeyPoint> keyPntsI, keyPntsJ;
	Mat descriptorsI, descriptorsJ;

	SurfFeatureDetector surf(SURF_MIN_HESSIAN);
	SurfDescriptorExtractor extractor;
	FlannBasedMatcher matcher;
    vector< vector<DMatch> > matches;
    vector< pair<vector<Point2d>, vector<Point2d> > > goodMatches;
    vector< Mat > transformGs;
    
	for (int f=0; f < calCountLimit; f++) {
		// Load images first
		(*videoI) >> frameI;
		(*videoJ) >> frameJ;

		// Find key points using SURF
        surf.detect(frameI, keyPntsI);
        surf.detect(frameJ, keyPntsJ);

        // Calculate descriptors
        extractor.compute(frameI, keyPntsI, descriptorsI);
        extractor.compute(frameJ, keyPntsJ, descriptorsJ);

        // Use Flann-Based Matcher to match
        matches.clear();
        matcher.knnMatch( descriptorsI, descriptorsJ, matches, 2);

        vector< Point2d > goodMatchesI;
    	vector< Point2d > goodMatchesJ;
        // Check if matches are good or not
        for (int m = matches.size()-1 ; m >= 0; m--){
            if (matches[m].size() < 2)
                continue;
            const DMatch &m1 = matches[m][0];
            const DMatch &m2 = matches[m][1];
            if (m1.distance <= MATCH_NNDRRATIO * m2.distance) {
            	goodMatchesI.push_back( keyPntsI[m1.queryIdx].pt );
            	goodMatchesJ.push_back( keyPntsJ[m1.trainIdx].pt );
            }
		}
		goodMatches.push_back(make_pair(goodMatchesI, goodMatchesJ));
		vector<unsigned char> inliersMask(goodMatchesI.size());
		Mat H = findHomography(goodMatchesI, goodMatchesJ, CV_RANSAC, RANSAC_REPROJ_THRES, inliersMask);
		transformGs.push_back( H );

		
		Mat tmpI;
		warpPerspective(frameI, tmpI, H, Size(frameJ.cols*2, frameJ.rows*2));
		namedWindow("matches_good",0);
		imshow("matches_good", tmpI);
		waitKey(0);


        logMsg(LOG_DEBUG, "SURF done for " + to_string(f) + " with matches: " + to_string(goodMatchesI.size()));
	}
	// Save the results
	FeatureMatchResult* fmr = new FeatureMatchResult();
	fmr->i = idxI;
	fmr->j = idxJ;
	fmr->matches = goodMatches;
	fmr->transformGs = transformGs;
	mFeatureMatchResults.push_back(fmr);
}

void RefProjGenerator::doReferenceProjection() {
	int videoAmount = mVL->getVideoListSize();
	for (int i=1; i<videoAmount; i++) {
		calMutualPosition(i-1, i);
	}
}

RefProjGenerator::RefProjGenerator(VideoLoader* videoLoader) {
	mVL = videoLoader;
}

RefProjGenerator::~RefProjGenerator() {
	
}