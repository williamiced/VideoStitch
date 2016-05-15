#include <header/SaliencyMapHandler.h>

using namespace std;
using namespace utils;
using namespace Array;
using namespace fftwpp;

void SaliencyMapHandler::loadSaliencyVideo(char* saliencyFileName) {
	using namespace cv;

	mSaliencyVideo = new VideoCapture(saliencyFileName);
	mW = (int) mSaliencyVideo->get(CV_CAP_PROP_FRAME_WIDTH);
	mH = (int) mSaliencyVideo->get(CV_CAP_PROP_FRAME_HEIGHT);

	mSaliencyReaderThread = thread(&SaliencyMapHandler::preloadSaliencyVideo, this);
}

void SaliencyMapHandler::analyzeInfo(cv::Mat img, cv::Mat& info) {
	using namespace cv;

	int h = mH / mGridSize;
	int w = mW / mGridSize;
	info = Mat::zeros(h, w, CV_8UC1);
	
	for (int y=0; y<h; y++) {
		for (int x=0; x<w; x++) {
			int total = 0;
			for (int y0=y*mGridSize; y0<(y+1)*mGridSize; y0++) {
				for (int x0=x*mGridSize; x0<(x+1)*mGridSize; x0++) {
					if (img.at<Vec3b>(y0, x0)[0] > mGridThresh)
						total++;
					else
						total--;
				}	
			}
			if (total >= 0)
				info.at<uchar>(y, x) = 255;
			else
				info.at<uchar>(y, x) = 0;
		}
	}
}

void SaliencyMapHandler::preloadSaliencyVideo() {
	using namespace cv;

	mIsProducerRun = true;

	while ( (int) mSaliencyBuffer.size() < mContainerSize && !mIsFinish) {
		Mat frame;
		
		if ( !mSaliencyVideo->read(frame) ) {
			logMsg(LOG_WARNING, stringFormat("Saliency Map frame is broken on idx: %d", mCurrentFirstFrame) , 4);
		} else {
			Mat saliencyInfo;
			analyzeInfo(frame, saliencyInfo);
			mBufLock.lock();
			mSaliencyBuffer.push(saliencyInfo);	
			mBufLock.unlock();
			logMsg(LOG_INFO, stringFormat("Read saliency frame #%d", mCurrentFirstFrame), 4);
		}
		
		mCurrentFirstFrame++;
		if (mCurrentFirstFrame >= mDuration)
			mIsFinish = true;
	}
	mIsProducerRun = false;	
}

bool SaliencyMapHandler::getSaliencyFrameFromVideo(cv::Mat& frame) {
	using namespace cv;

	if (isCleanup())
		return false;

	while (mSaliencyBuffer.size() == 0)	 {
		if (!mIsProducerRun)
			wakeLoaderUp();
		if (mIsFinish)
			break;
	}
	if (mSaliencyBuffer.size() == 0)
		return false;
	
	mBufLock.lock();
	frame = mSaliencyBuffer.front();
	mSaliencyBuffer.pop();
	mBufLock.unlock();
	wakeLoaderUp();

	return true;
}

bool SaliencyMapHandler::calculateSaliencyFromPQFT(cv::Mat& frame, cv::Mat& saliencyInfo) {
	SETUP_TIMER

	cv::Mat rawSaliency = cv::Mat::zeros(mFH, mFW, CV_64FC1);
	saliencyInfo = cv::Mat::zeros(mFH, mFW, CV_8UC1);
	cv::Mat smallFrame;
	cv::resize(frame, smallFrame, cv::Size(mFW, mFH));
	cv::Mat channels[3];
  	cv::split(smallFrame, channels);

  	for (int i=0; i<mFW * mFH; i++) {
		mC_r[i] = (int)channels[2].data[i];
		mC_g[i] = (int)channels[1].data[i];
		mC_b[i] = (int)channels[0].data[i];
	}

	mC_I = mI_record[mCurrentI % mTempCohQueueSize];

	float normalizeTotal = 1e-5;
	for (unsigned int i=0; i<min(mCurrentI, mTempCohQueueSize); i++)
		normalizeTotal += mGaussianWeights[i];
	normalizeTotal = 1.f / normalizeTotal;

	for (int i=0; i<mFW * mFH; i++) {
		mC_R[i] = mC_r[i] - (mC_g[i] + mC_b[i]) / 2;
		mC_G[i] = mC_g[i] - (mC_r[i] + mC_b[i]) / 2;
		mC_B[i] = mC_b[i] - (mC_r[i] + mC_g[i]) / 2;
		mC_Y[i] = (mC_r[i]+mC_g[i])/2 - abs(mC_r[i]-mC_g[i])/2 - mC_b[i];
		mC_RG[i] = mC_R[i]-mC_G[i];
		mC_BY[i] = mC_B[i]-mC_Y[i];
		mC_I[i] = (mC_r[i] + mC_g[i] + mC_b[i])/3;
	}

	memset(mC_M, 0, sizeof(int) * mFW * mFH);

	for (int j=mTempCohQueueSize-1, count=0; count<min(mCurrentI, mTempCohQueueSize-1); j--, count++) {
		float weight = mGaussianWeights[count] * normalizeTotal;
		for (int i=0; i<mFW * mFH; i++) 
			mC_M[i] += abs(mC_I[i] - mI_record[ (mCurrentI + j) % mTempCohQueueSize ][i]) * weight;
	}

	mCurrentI++;

	for(int i=0; i < mFW; i++) 
		for(int j=0; j < mFH; j++) 
			(*mf1)(i,j)=Complex(3 * mC_M[j*mFW+i],mC_RG[j*mFW+i]);

	for(int i=0; i < mFW; i++) 
		for(int j=0; j < mFH; j++) 
			(*mf2)(i,j)=Complex(mC_BY[j*mFW+i],mC_I[j*mFW+i]);

	mForward1->fft(*mf1);
	mForward2->fft(*mf2);
  
	for(int i=0; i < mFW; i++) {
		for(int j=0; j < mFH; j++) {
			double p0 = real((*mf1)(i,j));
			double p1 = imag((*mf1)(i,j));
			double p2 = real((*mf2)(i,j));
			double p3 = imag((*mf2)(i,j));
			double mag = sqrt(p0*p0 + p1*p1 + p2*p2 + p3*p3);
			(*mf1)(i,j)=Complex(real((*mf1)(i,j)) / mag, imag((*mf1)(i,j)) / mag);
			(*mf2)(i,j)=Complex(real((*mf2)(i,j)) / mag, imag((*mf2)(i,j)) / mag);
		}
	}

	mBackward1->fftNormalized(*mf1);
	mBackward2->fftNormalized(*mf2);

	for(int i=0; i < mFW; i++) 
		for(int j=0; j < mFH; j++) {
			double p0 = real((*mf1)(i,j));
			double p1 = imag((*mf1)(i,j));
			double p2 = real((*mf2)(i,j));
			double p3 = imag((*mf2)(i,j));
			double dist = p0*p0 + p1*p1 + p2*p2 + p3*p3;
			rawSaliency.at<double>(j, i) = dist;
		}

	cv::normalize(rawSaliency, saliencyInfo, 0, PQFT_SCALE, cv::NORM_MINMAX, CV_8UC1);
	//cv::resize(saliencyInfo, saliencyInfo, cv::Size(mFW/mGridSize, mFH/mGridSize));
	cv::threshold(saliencyInfo, saliencyInfo, PQFT_THRESH, 255, cv::THRESH_BINARY);

	int dilation_size = 3;
	cv::Mat kernel = cv::getStructuringElement (cv::MORPH_ELLIPSE, cv::Size( 2*dilation_size + 1, 2*dilation_size+1 ), cv::Point( dilation_size, dilation_size ));
	cv::dilate( saliencyInfo, saliencyInfo, kernel );

	int erode_size = 1;
	cv::Mat kernel2 = cv::getStructuringElement (cv::MORPH_ELLIPSE, cv::Size( 2*erode_size + 1, 2*erode_size+1 ), cv::Point( erode_size, erode_size ));
	for (int i=0; i<2; i++)
		erode( saliencyInfo, saliencyInfo, kernel2 );

  	//cv::GaussianBlur(saliencyInfo, saliencyInfo, cv::Size( 3, 3 ), 0, 0);

	return true;
}

bool SaliencyMapHandler::calculateSaliencyFromKLT(cv::Mat& frame, cv::Mat& saliencyInfo) {
	using namespace cv;

	SETUP_TIMER
	Mat featureCanvas;
	cv::resize(frame, featureCanvas, Size(mFW, mFH));
	mVVA->process(featureCanvas);

	mFeatureTrackers = mVVA->getActiveTrackers();

	logMsg(LOG_DEBUG, stringFormat("Current is oK, feature num is %d", mFeatureTrackers.size() )) ;

	getSaliencyInfoFromTrackers(saliencyInfo);

	return true;
}

void SaliencyMapHandler::getSaliencyInfoFromTrackers(cv::Mat& info) {
	using namespace cv;

	/*
	if (mCurrentFirstFrame % 5 != 0) {
		mCurrentFirstFrame++;
		info = mLastInfo;
		return;
	}
	*/

	int h = mFH / mGridSize;
	int w = mFW / mGridSize;

	info = Mat::zeros(h, w, CV_8UC1);

	float unit = 1.f / (mGridSize * mGridSize);

	Mat featureCountMat = Mat::zeros(h, w, CV_32FC1);

	for(FeatureTracker* tracker : mFeatureTrackers) {
		Point p = tracker->getLastPoint();
		int x = p.x / mGridSize;
		int y = p.y / mGridSize;

		featureCountMat.at<float>(y, x) += unit;
	}

	if ((int)mInfoVec.size() == mTempCohQueueSize)
		mInfoVec.erase(mInfoVec.begin());
	mInfoVec.push_back(featureCountMat.clone());

	memset(mFeatureCounter, 0, sizeof(float) * h*w);

	float normalizeTotal = 0.f;
	for (unsigned int i=0; i<mInfoVec.size(); i++)
		normalizeTotal += mGaussianWeights[i];
	normalizeTotal = 1.f / normalizeTotal;

	for (unsigned int i=0; i<mInfoVec.size(); i++) {
		Mat pastInfo = mInfoVec[i];
		for (int y=0; y<h; y++) {
			for (int x=0; x<w; x++) {
				mFeatureCounter[y*w+x] += pastInfo.at<float>(y, x) * mGaussianWeights[mInfoVec.size()-1 - i] * normalizeTotal;
			}
		}
	}

	for (int y=0; y<h; y++) 
		for (int x=0; x<w; x++) 
			info.at<uchar>(y, x) = mFeatureCounter[y*w+x] >= mThreshKLT ? 255 : 0;

  	/// Apply the dilation operation
	
	int dilation_size = 5;
	Mat kernel = getStructuringElement (MORPH_ELLIPSE, Size( 2*dilation_size + 1, 2*dilation_size+1 ), Point( dilation_size, dilation_size ));
	dilate( info, info, kernel );

	int erode_size = 1;
	Mat kernel2 = getStructuringElement (MORPH_ELLIPSE, Size( 2*erode_size + 1, 2*erode_size+1 ), Point( erode_size, erode_size ));
	for (int i=0; i<4; i++)
		erode( info, info, kernel2 );

	mLastInfo = info;
	
	mCurrentFirstFrame++;
}

bool SaliencyMapHandler::wakeLoaderUp() {
	if (!mIsProducerRun) {
		if (mSaliencyReaderThread.joinable())
			mSaliencyReaderThread.join();
		
		if (mIsFinish)
			return false;
		mSaliencyReaderThread = thread(&SaliencyMapHandler::preloadSaliencyVideo, this);
	}
	return true;
}

bool SaliencyMapHandler::isCleanup() {
	return mIsFinish && (mSaliencyBuffer.size() == 0);
}

void SaliencyMapHandler::initPQFT() {
	mC_r = new int[mFW * mFH];
	mC_g = new int[mFW * mFH];
	mC_b = new int[mFW * mFH];
	mC_R = new int[mFW * mFH];
	mC_G = new int[mFW * mFH];
	mC_B = new int[mFW * mFH];
	mC_Y = new int[mFW * mFH];
	mC_RG = new int[mFW * mFH];
	mC_BY = new int[mFW * mFH];
	mC_M = new int[mFW * mFH];

	for (int i=0; i<mTempCohQueueSize; i++) {
		mI_record.push_back(new int[mFW * mFH]);
		memset(mI_record[i], 0, sizeof(int) * mFW * mFH);
	}
	mCurrentI = 0;

	fftw::maxthreads = get_max_threads();
	size_t align = sizeof(Complex);
	mf1 = new array2<Complex>(mFW, mFH, align);
	mf2 = new array2<Complex>(mFW, mFH, align);
  	mForward1 = new fft2d(-1, *mf1);
  	mBackward1 = new fft2d(1, *mf1);
  	mForward2 = new fft2d(-1, *mf2);
  	mBackward2 = new fft2d(1, *mf2);
}

SaliencyMapHandler::SaliencyMapHandler(char* saliencyFileName, int duration) : 
	mCurrentFirstFrame(0), 
	mIsProducerRun(false), 
	mIsFinish(false), 
	mDuration(duration),
 	mGridSize(getIntConfig("SALIENCY_GRID_SIZE")),
 	mGridThresh(getIntConfig("EPSILON_F")),
 	mContainerSize(getIntConfig("VIDEO_CONTAINER_SIZE")) {

	loadSaliencyVideo(saliencyFileName);
}

SaliencyMapHandler::SaliencyMapHandler(bool isKLT) :
	mCurrentFirstFrame(0),
	mGridSize(getIntConfig("SALIENCY_GRID_SIZE")),
 	mThreshKLT(getFloatConfig("EPSILON_F_KLT")),
 	mFW(getIntConfig("FEATURE_CANVAS_WIDTH")),
 	mFH(getIntConfig("FEATURE_CANVAS_HEIGHT")),
 	mTempCohQueueSize(getIntConfig("TEMP_COH_QUEUE_SIZE")),
	mTempCohSigma(getFloatConfig("TEMP_COH_SIGMA")),
	mIsKLT(isKLT) {

 	int h = mFH / mGridSize;
	int w = mFW / mGridSize;
	mFeatureCounter = new float[h*w];

	mTemCohFactor1 = 1 / sqrt(2 * M_PI * mTempCohSigma * mTempCohSigma);
	mTemCohFactor2 = 1 / (2 * mTempCohSigma * mTempCohSigma);

	for (int i=0; i<mTempCohQueueSize; i++) 
		mGaussianWeights.push_back(mTemCohFactor1 * exp((-1) * i * i * mTemCohFactor2));

	if (mIsKLT) // KLT
		mVVA = unique_ptr<VideoVolumeAnalyzer>( new VideoVolumeAnalyzer() );
	else // PQFT
		initPQFT();
}

SaliencyMapHandler::~SaliencyMapHandler() {
	if (mSaliencyVideo != nullptr)
		mSaliencyVideo->release();
	if (mVVA != nullptr)
		mVVA.release();
	if (mFeatureCounter != nullptr) {
		delete mFeatureCounter;
	}

	if (mC_r != nullptr) {
		delete[] mC_r;
		delete[] mC_g;
		delete[] mC_b;
		delete[] mC_R;
		delete[] mC_G;
		delete[] mC_B;
		delete[] mC_Y;
		delete[] mC_RG;
		delete[] mC_BY;
		delete[] mC_M;

		for (int i=0; i<mTempCohQueueSize; i++)
			delete[] mI_record[i];

		delete mf1;
		delete mf2;
		delete mForward1;
		delete mForward2;
		delete mBackward1;
		delete mBackward2;
	}
}