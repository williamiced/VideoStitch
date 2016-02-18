#include <header/MappingProjector.h>

Size MappingProjector::calcProjectionMatrix() {

#ifdef USE_SMALLER_CANVAS
	logMsg(LOG_INFO, "=== Shrink canvas to half ===");
	mViewSize /= USE_SMALLER_CANVAS;
	logMsg(LOG_DEBUG, stringFormat("\tNew Focal Length: %lf", mFocalLength));
#endif
	// Calculate mR
	if (mR.size() == 0) {
		logMsg(LOG_INFO, "=== Calculate rotation matrix for each view ===");
		calcRotationMatrix();
	} else {
		logMsg(LOG_INFO, "=== Recalculate the rotation matrix ===");
		//recalcRotationMatrix();
	}

	// Calculate mK
	if (mK.size() == 0) {
		logMsg(LOG_INFO, "=== Calculate intrinsic matrix for each view ===");
		mFocalLength /= static_cast<float>( USE_SMALLER_CANVAS );
		calcIntrinsicMatrix();
	} else {
		for (int v=0; v<mViewCount; v++) {
			mK[v].at<float>(0, 0) /= USE_SMALLER_CANVAS;
			mK[v].at<float>(1, 1) /= USE_SMALLER_CANVAS;
			mK[v].at<float>(0, 2) /= USE_SMALLER_CANVAS;
			mK[v].at<float>(1, 2) /= USE_SMALLER_CANVAS;
		}
	}

	for (int v=0; v<mViewCount; v++) {
		float scale = (mK[v].at<float>(0, 0) + mK[v].at<float>(1, 1))/2 ;
		logMsg(LOG_DEBUG, stringFormat("Project scale: %f", scale) );
		mSphericalWarpers.push_back( shared_ptr<PROJECT_METHOD>( new PROJECT_METHOD( scale ) ) );
	}
	logMsg(LOG_INFO, "=== Start to construct the sphere map ===");
	constructSphereMap();
	return mCanvasROI.size();
}

void MappingProjector::constructSphereMap() {
	// Build the maps
	logMsg(LOG_INFO, "=== Build maps for each views ===");
	findingMappingAndROI();

	// Construct masks
	logMsg(LOG_INFO, "=== Build maps masks ===");
	constructMasks();

#ifndef USE_BLENDER
	// Construct alpha channels
	logMsg(LOG_INFO, "=== Construct alpha channels ===");
	constructAlphaChannel();
#endif
}

void MappingProjector::calcRotationMatrix() {
	for (int v=0; v<mViewCount; v++) {
		Mat r = Mat::zeros(3, 3, CV_32F);
		struct MutualProjectParam vP = mViewParams[v];
		double alpha = vP.y;
		double beta = vP.p;
		double gamma = vP.r;

		// Take camera as reference coordinate system, around: x-axis -> pitch, y-axis -> yaw, z->axis -> roll
		Mat Rz = getZMatrix(gamma);
		Mat Ry = getYMatrix(alpha);
		Mat Rx = getXMatrix(beta);
		// r = Ry * Rz * Rx;
		r = Ry * Rx * Rz;
		mR.push_back( r );
	}
}

void MappingProjector::recalcRotationMatrix() {
	for (int v=0; v<mViewCount; v++) {
		Mat r = Mat::zeros(3, 3, CV_32F);
		Mat ro = mR[v];
		float alpha = atan2(ro.at<float>(1, 0), ro.at<float>(0, 0));
		float beta = atan2( -ro.at<float>(2, 0), sqrt( pow(ro.at<float>(2, 1), 2) + pow(ro.at<float>(2, 2), 2) ) );
		float gamma = atan2(ro.at<float>(2, 1), ro.at<float>(2, 2));

		// Take camera as reference coordinate system, around: x-axis -> pitch, y-axis -> yaw, z->axis -> roll
		Mat Rz = getZMatrix(gamma);
		Mat Ry = getYMatrix(alpha);
		Mat Rx = getXMatrix(beta);
		// r = Ry * Rz * Rx;
		r = Ry * Rx * Rz;
		mR[v] = r;
	}
}

void MappingProjector::calcIntrinsicMatrix() {
	mK.resize(mViewCount);
	for (int v=0; v<mViewCount; v++) {
		mK[v] = Mat::zeros(3, 3, CV_32F);
		// Refresh focal length from pto file
		mK[v].at<float>(0, 0) = static_cast<float> ( mFocalLength );
		mK[v].at<float>(1, 1) = static_cast<float> ( mFocalLength );
		mK[v].at<float>(0, 2) = static_cast<float> ( mViewSize.width/2 );
		mK[v].at<float>(1, 2) = static_cast<float> ( mViewSize.height/2 );
		mK[v].at<float>(2, 2) = 1.f;
	}
}

void MappingProjector::findingMappingAndROI() {
	for (int v=0; v<mViewCount; v++) {
		Mat uxmap, uymap;
		mMapROIs.push_back( mSphericalWarpers[v]->buildMaps(mViewSize, mK[v], mR[v], uxmap, uymap) );
		mUxMaps.push_back( uxmap );
		mUyMaps.push_back( uymap );
	}
	updateCurrentCanvasROI();

	// Update ROIs
	if (mCanvasROI.size().width < mCanvasROI.size().height * 2) {
		int yOffset = mCanvasROI.size().width / 4;
		for (int v=0; v<mViewCount; v++) {
			Point tl = Point(mMapROIs[v].tl().x, mMapROIs[v].tl().y < -yOffset ? -yOffset : mMapROIs[v].tl().y);
			Point br = Point(mMapROIs[v].br().x, mMapROIs[v].br().y > yOffset ? yOffset : mMapROIs[v].br().y );
			mUxMaps[v] = mUxMaps[v].rowRange( tl.y - mMapROIs[v].tl().y, br.y - mMapROIs[v].tl().y + 1);
			mUyMaps[v] = mUyMaps[v].rowRange( tl.y - mMapROIs[v].tl().y, br.y - mMapROIs[v].tl().y + 1);
			mMapROIs[v] = Rect(tl, br);
		}

		updateCurrentCanvasROI();
	}

	for (int v=0; v<mViewCount; v++) {
		mMapROIs[v] = Rect( mMapROIs[v].x - mCanvasROI.x, mMapROIs[v].y - mCanvasROI.y, mMapROIs[v].width+1, mMapROIs[v].height+1 );
		mCorners.push_back( mMapROIs[v].tl() );
	}
}

void MappingProjector::updateCurrentCanvasROI() {
	// Calculate canvas ROI
	mCanvasROI = mMapROIs[0];
	for (int v=1; v<mViewCount; v++)
		mCanvasROI = mCanvasROI | mMapROIs[v];
	mCanvasROI += Size(1, 1);
}

void MappingProjector::constructMasks() {
	Mat sampleMat(mViewSize, CV_8U, Scalar(255));
	
	for (int v=0; v<mViewCount; v++) {
		// Construct whole Mat
		Mat maskFullView = Mat::zeros(mMapROIs[v].height + 1, mMapROIs[v].width + 1, CV_8U);
		cv::remap(sampleMat, maskFullView, mUxMaps[v], mUyMaps[v], cv::INTER_LINEAR, BORDER_CONSTANT);
		mMapMasks.push_back(maskFullView);
	}
}

void MappingProjector::constructAlphaChannel() {
	mAlphaChannel = Mat(mCanvasROI.size(), CV_8U, Scalar(0));
	for (int v=0; v<mViewCount; v++) {
		Mat viewRange = mAlphaChannel( mMapROIs[v] );
		viewRange += (mMapMasks[v] / 255);
	}
	//cout << mAlphaChannel << endl;
	for (int v=0; v<mViewCount; v++) {
		Mat viewAlpha;
		Mat viewRange;
		mAlphaChannel( mMapROIs[v] ).convertTo(viewRange, CV_32FC1);
		mMapMasks[v].convertTo(viewAlpha, CV_32FC1, 1/255.0f);

		Mat result;
		divide(viewAlpha, viewRange, result);
		result.convertTo(result, CV_32FC1);
		mViewAlpha.push_back(result);
	}
}

Mat MappingProjector::getZMatrix(double alpha) {
	Mat z = Mat::zeros(3, 3, CV_32F);
	float cosz = cos(alpha);
	float sinz = sin(alpha);
	z.at<float>(0, 0) = cosz;
	z.at<float>(0, 1) = -sinz;
	z.at<float>(1, 0) = sinz;
	z.at<float>(1, 1) = cosz;
	z.at<float>(2, 2) = 1.f;
	return z;
}

Mat MappingProjector::getYMatrix(double beta) {
	Mat y = Mat::zeros(3, 3, CV_32F);
	float cosy = cos(beta);
	float siny = sin(beta);
	y.at<float>(0, 0) = cosy;
	y.at<float>(0, 2) = siny;
	y.at<float>(1, 1) = 1.f;
	y.at<float>(2, 0) = -siny;
	y.at<float>(2, 2) = cosy;
	return y;
}

Mat MappingProjector::getXMatrix(double gamma) {
	Mat x = Mat::zeros(3, 3, CV_32F);
	float cosx = cos(gamma);
	float sinx = sin(gamma);
	x.at<float>(0, 0) = 1.f;
	x.at<float>(1, 1) = cosx;
	x.at<float>(1, 2) = -sinx;
	x.at<float>(2, 1) = sinx;
	x.at<float>(2, 2) = cosx;
	return x;
}

void MappingProjector::projectOnCanvas(Mat& canvas, vector<Mat> frames) {
	boost::timer::cpu_timer boostTimer;

	vector<Mat> warpedImg(mViewCount);
#ifdef USE_SMALLER_CANVAS
	canvas = Mat::zeros(mCanvasROI.size(), CV_8UC3);
#endif
	for (int v=0; v<mViewCount; v++) {
#ifdef USE_SMALLER_CANVAS
		cv::resize(frames[v], frames[v], mViewSize);
#endif
		// Get the output frame
		Mat outputFrame(mMapROIs[v].height + 1, mMapROIs[v].width + 1, CV_8UC3);
		cv::remap(frames[v], outputFrame, mUxMaps[v], mUyMaps[v], cv::INTER_LINEAR, BORDER_CONSTANT);
		warpedImg[v] = outputFrame;
	}

#ifdef USE_EXPOSURE_COMPENSATOR
	if (mEP == nullptr)
		mEP = shared_ptr<ExposureProcessor>( new ExposureProcessor( mCorners, mMapMasks, mViewCount ) );
	if (mFrameProcessed == 0)
		mEP->feedExposures(warpedImg);
	mEP->doExposureCompensate(warpedImg);
#endif

	Mat result, resultMask;
#ifdef USE_BLENDER
	if (mBP == nullptr) {
		vector<Size> sizes(mViewCount);
		for (int v=0; v<mViewCount; v++)
			sizes[v] = mMapROIs[v].size();
		mBP = shared_ptr<BlendingProcessor>( new BlendingProcessor( mViewCount, mCanvasROI, mCorners, sizes, mMapMasks ) );
	}
	mBP->doBlending( warpedImg, result, resultMask );
	canvas = result;
#else
	for (int v=0; v<mViewCount; v++) {
		mixWithAlphaChannel(warpedImg[v], v);
		add( canvas( mMapROIs[v] ), warpedImg[v], canvas( mMapROIs[v] ), mMapMasks[v]);	
	}
#endif

	boostTimer.stop();
	mExecTimes.push_back( stod(boostTimer.format(3, "%w")) );
    mFrameProcessed++;
}

void MappingProjector::setCameraParams( vector<Mat> Rs, vector<Mat> Ks, vector<Mat> Ds ) {
	if ( !Rs.empty() )
		mR = Rs;
	if ( !Ds.empty() )
		mD = Ds;
	if ( !Ks.empty() ) 
		mK = Ks;
}

void MappingProjector::checkFPS() {
	double total = 0.f;
	for (unsigned int i=0; i<mExecTimes.size(); i++) 
		total += mExecTimes[i];
	logMsg(LOG_INFO, stringFormat( "=== Average FPS is %lf === ", mExecTimes.size() / total ) );
}

void MappingProjector::mixWithAlphaChannel(Mat& img, int v) {
	for (int y=0; y<img.rows; y++) {
		for (int x=0; x<img.cols; x++) {
			Vec3b data = img.at<Vec3b>(y, x);
			data *= mViewAlpha[v].at<float>(y, x);
			img.at<Vec3b>(y, x) = data;
		}
	}
}

MappingProjector::MappingProjector(int viewCount, Size viewSize, vector<struct MutualProjectParam> params, double focalLength) : mFrameProcessed(0) {
	mViewCount = viewCount;
	mViewSize = viewSize;
	mViewParams = params;
	mFocalLength = focalLength;

	logMsg(LOG_DEBUG, stringFormat("\tFocal Length in PTO: %lf", mFocalLength));
	for (int v=0; v<mViewCount; v++)
		logMsg(LOG_DEBUG, stringFormat("\t\tV%d: Yaw: %lf\t, Pitch: %lf\t, Roll: %lf", v, params[v].y, params[v].p, params[v].r));
}

MappingProjector::~MappingProjector() {
	
}
