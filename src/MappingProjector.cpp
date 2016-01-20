#include <header/MappingProjector.h>

Size MappingProjector::calcProjectionMatrix(map< string, Mat > calibrationData) {
	mA = calibrationData["cameraMatA"];
	mD = calibrationData["distCoeffs"];

	// Refresh focal length from pto file
	mA.at<float>(0, 0) = static_cast<float> ( mFocalLength );
	mA.at<float>(1, 1) = static_cast<float> ( mFocalLength );
	mA.at<float>(0, 2) = static_cast<float> ( mViewSize.width/2 );
	mA.at<float>(1, 2) = static_cast<float> ( mViewSize.height/2 );

	mSphericalWarper = shared_ptr<cv::detail::SphericalWarper>( new cv::detail::SphericalWarper( mFocalLength ) );
	logMsg(LOG_INFO, "=== Start to construct the sphere map ===");
	constructSphereMap();
	return mCanvasROI.size();
}

void MappingProjector::constructSphereMap() {
	// Calculate mR
	logMsg(LOG_INFO, "=== Calculate projection matrix for each view ===");
	for (int v=0; v<mViewCount; v++) {
		Mat r = Mat::zeros(3, 3, CV_32F);
		if (mDebugView.find(v) == mDebugView.end()) {
			mR.push_back( r );
			continue;
		}
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

	// Build the maps
	logMsg(LOG_INFO, "=== Build maps for each views ===");
	for (int v=0; v<mViewCount; v++) {
		if (mDebugView.find(v) == mDebugView.end()) {
			mMapROIs.push_back( Rect(0, 0, 0, 0) );
			mUxMaps.push_back( UMat() );
			mUyMaps.push_back( UMat() );
			continue;
		}
		UMat uxmap, uymap;
		mMapROIs.push_back( mSphericalWarper->buildMaps(mViewSize, mA, mR[v], uxmap, uymap) );
		mUxMaps.push_back( uxmap );
		mUyMaps.push_back( uymap );

		//cout << "ROI: " << mMapROIs[v].tl() << " -> " << mMapROIs[v].br() << endl;
	}

	mCanvasROI = mMapROIs[0];
	for (int v=1; v<mViewCount; v++)
		mCanvasROI = mCanvasROI | mMapROIs[v];
	mCanvasROI += Size(1, 1);

	// Update ROIs
	for (int v=0; v<mViewCount; v++) 
		mMapROIs[v] = Rect( mMapROIs[v].x - mCanvasROI.x, mMapROIs[v].y - mCanvasROI.y, mMapROIs[v].width+1, mMapROIs[v].height+1 );
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
	for (int v=0; v<mViewCount; v++) {
		// Get the output frame
		Mat outputFrame;
		outputFrame.create(mMapROIs[v].height + 1, mMapROIs[v].width + 1, frames[v].type());

		cv::remap(frames[v], outputFrame, mUxMaps[v], mUyMaps[v], cv::INTER_LINEAR, BORDER_CONSTANT);
		//outputFrames.push_back(outputFrame);

		if ( mMapMasks.size() < (unsigned int)mViewCount ) { // Construct maps for the first frame
			// Generate mask for map
			Mat binaryOutputFrame;
			cvtColor(outputFrame, binaryOutputFrame, CV_RGB2GRAY);
			Mat mask(outputFrame.size(), CV_8U, Scalar(0));
			vector<vector<Point> > contours;
			vector<Vec4i> hierarchy;
			findContours(binaryOutputFrame, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
			for (int i=0; i>=0; i = hierarchy[i][0]) 
				drawContours(mask, contours, i, Scalar(255), CV_FILLED);
			mMapMasks.push_back(mask);
		}

		outputFrame.copyTo(canvas( mMapROIs[v] ), mMapMasks[v]);
	}
}


MappingProjector::MappingProjector(int viewCount, Size viewSize, vector<struct MutualProjectParam> params, double focalLength) {
	mViewCount = viewCount;
	mViewSize = viewSize;
	mViewParams = params;
	mFocalLength = focalLength;
	mDebugView = {0, 1, 2, 3, 4, 5};

	logMsg(LOG_DEBUG, stringFormat("\tFocal Length: %lf", mFocalLength));
	for (int v=0; v<mViewCount; v++)
		logMsg(LOG_DEBUG, stringFormat("\t\tV%d: Yaw: %lf\t, Pitch: %lf\t, Roll: %lf", v, params[v].y, params[v].p, params[v].r));

}

MappingProjector::~MappingProjector() {
	
}
