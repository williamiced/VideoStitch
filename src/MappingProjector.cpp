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
	calcRotationMatrix();
	
	// Build the maps
	logMsg(LOG_INFO, "=== Build maps for each views ===");
	findingMappingAndROI();

	// Construct masks
	constructMasks();

	// Construct alpha channels
	constructAlphaChannel();
}

void MappingProjector::calcRotationMatrix() {
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
}

void MappingProjector::findingMappingAndROI() {
	for (int v=0; v<mViewCount; v++) {
		if (mDebugView.find(v) == mDebugView.end()) {
			mMapROIs.push_back( Rect(0, 0, 0, 0) );
			mUxMaps.push_back( Mat() );
			mUyMaps.push_back( Mat() );
			continue;
		}
		Mat uxmap, uymap;
		mMapROIs.push_back( mSphericalWarper->buildMaps(mViewSize, mA, mR[v], uxmap, uymap) );
		mUxMaps.push_back( uxmap );
		mUyMaps.push_back( uymap );
	}

	// Calculate canvas ROI
	mCanvasROI = mMapROIs[0];
	for (int v=1; v<mViewCount; v++)
		mCanvasROI = mCanvasROI | mMapROIs[v];
	mCanvasROI += Size(1, 1);

	// Update ROIs
	for (int v=0; v<mViewCount; v++) 
		mMapROIs[v] = Rect( mMapROIs[v].x - mCanvasROI.x, mMapROIs[v].y - mCanvasROI.y, mMapROIs[v].width+1, mMapROIs[v].height+1 );
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
		//imwrite(stringFormat("tmp_%d.png", v), result);
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
	for (int v=0; v<mViewCount; v++) {
		// Get the output frame
		Mat outputFrame;
		outputFrame.create(mMapROIs[v].height + 1, mMapROIs[v].width + 1, CV_8UC3);

		cv::remap(frames[v], outputFrame, mUxMaps[v], mUyMaps[v], cv::INTER_LINEAR, BORDER_CONSTANT);
		mixWithAlphaChannel(outputFrame, v);
		imwrite("tmpView.png", outputFrame);
		add(outputFrame, canvas( mMapROIs[v] ), canvas( mMapROIs[v] ), mMapMasks[v]);
	}
	imwrite("tmpCanvas.png", canvas);
}

void MappingProjector::mixWithAlphaChannel(Mat& img, int v) {
	vector<Mat> channels;
	split(img, channels);
	for (unsigned int i=0; i<channels.size(); i++) {
		channels[i].convertTo(channels[i], CV_32FC1);
		channels[i] = channels[i].mul(mViewAlpha[v]);
		channels[i].convertTo(channels[i], CV_8UC1);
	}
	merge(channels, img);
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
