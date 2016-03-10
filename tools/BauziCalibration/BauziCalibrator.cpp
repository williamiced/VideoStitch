#include "BauziCalibrator.h"

int BauziCalibrator::loadImages(char* fileName) {
	ifstream inputFile(fileName);

	string imageName;
	while (getline(inputFile, imageName)) {
		Mat img;
		img = imread(imageName);
		mRawImages.push_back(img);
	}
	return mRawImages.size();
}

void BauziCalibrator::doFeatureMatching() {
	//-- Step 1: Detect the keypoints using SURF Detector
	int minHessian = 400;

	for (int i=0; i<mImageCount; i++) {
		for (int j=i+1; j<mImageCount; j++) {
			// Feature Matching Algorithm
			Ptr<SIFT> FMA = SIFT::create( minHessian ) ;

			std::vector<KeyPoint> keypoints_1, keypoints_2;
			FMA->detect( mRawImages[i], keypoints_1 );
			FMA->detect( mRawImages[j], keypoints_2 );

			Mat descriptors_1, descriptors_2;
			FMA->compute( mRawImages[i], keypoints_1, descriptors_1 );
			FMA->compute( mRawImages[j], keypoints_2, descriptors_2 );

			FMA.release();

			//-- Step 3: Matching descriptor vectors using FLANN matcher
			FlannBasedMatcher matcher;

			vector< vector< DMatch>  > matches;
			vector< DMatch > good_matches;
			matcher.knnMatch( descriptors_1, descriptors_2, matches, 2);

			float NNDRRATIO = 0.5;

			MatchInfo matchInfo;
			matchInfo.idx1 = i;
			matchInfo.idx2 = j;

			for( unsigned int i = 0 ; i < matches.size(); i++){
		        if (matches[i].size() < 2)
		                continue;
		        const DMatch &m1 = matches[i][0];
		        const DMatch &m2 = matches[i][1];
		        if (m1.distance <= NNDRRATIO * m2.distance) {
					good_matches.push_back(m1);
					FeatureMatch fm;
					fm.p1 = Point( keypoints_1[m1.queryIdx].pt.x, keypoints_1[m1.queryIdx].pt.y );
					fm.p2 = Point( keypoints_2[m1.trainIdx].pt.x, keypoints_2[m1.trainIdx].pt.y );
					matchInfo.matches.push_back(fm);
				}
			}

			mMatchInfos.push_back(matchInfo);
			/*
			if (good_matches.size() > 0) {
				Mat img_matches;
				drawMatches( mRawImages[i], keypoints_1, mRawImages[j], keypoints_2,
			               good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
			               vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

				//-- Show detected matches
				resize(img_matches, img_matches, img_matches.size() /2 );
				imshow( "Good Matches", img_matches );

				waitKey(0);
			}
			*/

			cout << stringFormat("Matching between %d and %d: %d", i+1, j+1, good_matches.size()) << endl;
		}
	}
}

void BauziCalibrator::genInitGuess() {
	mEPSet = unique_ptr<ExtrinsicParamSet>( new ExtrinsicParamSet(mImageCount) );

	/** Hard coded initial guess, should be modified if stable */
	ExtrinsicParam ep1(0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
	ExtrinsicParam ep2(0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
	ExtrinsicParam ep3(0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
	ExtrinsicParam ep4(0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
	ExtrinsicParam ep5(0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
	ExtrinsicParam ep6(0.f, 0.f, 0.f, 0.f, 0.f, 0.f);

	mEPSet->setParam( 0, ep1 );
	mEPSet->setParam( 1, ep2 );
	mEPSet->setParam( 2, ep3 );
	mEPSet->setParam( 3, ep4 );
	mEPSet->setParam( 4, ep5 );
	mEPSet->setParam( 5, ep6 );
}

void BauziCalibrator::findBestParams(int iterationCount) {
	// First time
	vector<ExtrinsicParamSet> possibleSets;
	mEPSet->generatePerturbation(10000, possibleSets);
	
	vector<double> scores;
	evaluateError(possibleSets, scores);
	
	vector<ExtrinsicParamSet> candidateSet;
	chooseCandidateSet(possibleSets, scores, candidateSet, 10);

	for (int i=0; i<iterationCount; i++) {
		possibleSets.clear();
		scores.clear();
		
		for (int j=0; j<10; j++)
			candidateSet[j].generatePerturbation(1000, possibleSets);

		evaluateError(possibleSets, scores);

		candidateSet.clear();
		chooseCandidateSet(possibleSets, scores, candidateSet, 10);
	}

	ExtrinsicParamSet bestEP = candidateSet[0]; // Top one
}

void BauziCalibrator::evaluateError(vector<ExtrinsicParamSet> possibleSets, vector<double>& scores) {
	scores.resize(possibleSets.size());
	for (int i=0; i<possibleSets.size(); i++) 
		scores[i] = getScore(possibleSets[i]);
}

double BauziCalibrator::getScore(ExtrinsicParamSet eps) {
	double score = 0.f;
	for (int i=0; i<mMatchInfos.size(); i++) {
		int idx1 = mMatchInfos[i].idx1;
		int idx2 = mMatchInfos[i].idx2;
		vector<FeatureMatch> matches = mMatchInfos[i].matches;

		for (int j=0; j<matches.size(); j++) {
			Point2f p1 = mSW->warpPoint( Point2f(matches[j].p1.x, matches[j].p1.y), mK[idx1], eps.params[idx1].getR() );
			Point2f p2 = mSW->warpPoint( Point2f(matches[j].p2.x, matches[j].p2.y), mK[idx2], eps.params[idx2].getR() );

			score += sqrt( pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2) );
		}
	}

	return 1.f / score;
}

void BauziCalibrator::chooseCandidateSet(vector<ExtrinsicParamSet> possibleSets, vector<double> scores, vector<ExtrinsicParamSet>& candidateSet, int chooseAmount) {
	int* indexArr = new int[scores.size()];

	iota(indexArr, indexArr + scores.size(), 0);

	sort(indexArr, indexArr + scores.size(), bind( 
		[] (int a, int b, vector<double> data) { return data[a] < data[b]; } 
	 	,  _1, _2, scores ));

	for (int i=0; i<chooseAmount; i++) 
		candidateSet.push_back( possibleSets[ indexArr[i] ] );

	delete[] indexArr;
}

BauziCalibrator::BauziCalibrator(char* fileName, int iterationCount) {
	mImageCount = loadImages(fileName);
	if (mImageCount < 2)
		logMsg(LOG_ERROR, stringFormat("Too few input images: %d", mImageCount ));

	mSW = unique_ptr<cv::detail::SphericalWarper>(new cv::detail::SphericalWarper( 1.f ) );
	mK.resize(mImageCount);
	for (int v=0; v<mImageCount; v++) {
		mK[v] = Mat::zeros(3, 3, CV_32F);
		mK[v].at<float>(0, 0) = static_cast<float> ( 640 );
		mK[v].at<float>(1, 1) = static_cast<float> ( 640 );
		mK[v].at<float>(0, 2) = static_cast<float> ( mRawImages[v].size().width/2 );
		mK[v].at<float>(1, 2) = static_cast<float> ( mRawImages[v].size().height/2 );
		mK[v].at<float>(2, 2) = 1.f;
	}

	doFeatureMatching();
	genInitGuess();
	findBestParams(iterationCount);
}

int main(int argc, char** argv) {
	if ( !checkArguments(argc, argv) )
		exitWithMsg(E_BAD_ARGUMENTS);

	unique_ptr<BauziCalibrator> bc = unique_ptr<BauziCalibrator>(new BauziCalibrator( getCmdOption(argv, argv + argc, "--input"), stoi(getCmdOption(argv, argv + argc, "--iter") )) );
}