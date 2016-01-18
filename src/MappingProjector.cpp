#include <header/MappingProjector.h>

void MappingProjector::calcProjectionMatrix(map< string, Mat > calibrationData) {
	calibrationData["cameraMatA"].convertTo(mA, CV_32F);
	mD = calibrationData["distCoeffs"];

	// Refresh focal length from pto file
	mA.at<float>(0, 0) = static_cast<float> ( mFocalLength );
	mA.at<float>(1, 1) = static_cast<float> ( mFocalLength );

	mSphericalWarper = shared_ptr<cv::detail::SphericalWarperGpu>( new cv::detail::SphericalWarperGpu(1.f) );
	constructSphereMap();
}

void MappingProjector::constructSphereMap() {
	// Initialize the map
	for (int y=0; y<OUTPUT_PANO_HEIGHT; y++) {
		vector<Mat> weightMatRow;
		for (int x=0; x<OUTPUT_PANO_WIDTH; x++) {
			Mat weightMat = Mat::zeros(mViewCount, 3, CV_32S);
			weightMatRow.push_back(weightMat);
		}
		mProjMap.push_back(weightMatRow);
	}

	// Calculate mR
	for (int v=0; v<mViewCount; v++) {
		if (mDebugView >= 0 && mDebugView != v)
			continue;
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


	// Calculate (x, y) -> (u, v) for each view
	for (int y=0; y<mViewSize.height; y++) {
		for (int x=0; x<mViewSize.width; x++) {
			for (int v=0; v<mViewCount; v++) {
				if (mDebugView >= 0 && mDebugView != v)
					continue;
				Point2f mapPnt = mSphericalWarper->warpPoint(Point2f(x, y), mA, mR[v]);
				// mapPnt: x = (-pi ~ pi), y = (0 ~ pi)
				int oX = static_cast<int> ( (mapPnt.x - (-M_PI)) * OUTPUT_PANO_WIDTH / (2 * M_PI) ) % OUTPUT_PANO_WIDTH;
				int oY = static_cast<int> ( mapPnt.y * OUTPUT_PANO_HEIGHT / (M_PI) )  % OUTPUT_PANO_HEIGHT;

				//cout << "map: (" << oX << ", " << oY << ") ... to v ( " << x << ", " << y << ")" << endl;

				Mat weightMat = mProjMap[oY][oX];
				weightMat.at<int>(v, 0) = 1;
				weightMat.at<int>(v, 1) = x;
				weightMat.at<int>(v, 2) = y;
				mProjMap[oY][oX] = weightMat;
			}
		}
	}
}

Mat MappingProjector::getZMatrix(double alpha) {
	Mat z = Mat::zeros(3, 3, CV_32F);
	z.at<float>(0, 0) = cos(alpha);
	z.at<float>(0, 1) = -sin(alpha);
	z.at<float>(1, 0) = sin(alpha);
	z.at<float>(1, 1) = cos(alpha);
	z.at<float>(2, 2) = 1.f;
	return z;
}

Mat MappingProjector::getYMatrix(double beta) {
	Mat y = Mat::zeros(3, 3, CV_32F);
	y.at<float>(0, 0) = cos(beta);
	y.at<float>(0, 2) = sin(beta);
	y.at<float>(1, 1) = 1.f;
	y.at<float>(2, 0) = -sin(beta);
	y.at<float>(2, 2) = cos(beta);
	return y;
}

Mat MappingProjector::getXMatrix(double gamma) {
	Mat x = Mat::zeros(3, 3, CV_32F);
	x.at<float>(0, 0) = 1.f;
	x.at<float>(1, 1) = cos(gamma);
	x.at<float>(1, 2) = -sin(gamma);
	x.at<float>(2, 1) = sin(gamma);
	x.at<float>(2, 2) = cos(gamma);
	return x;
}

void MappingProjector::projectOnCanvas(Mat& canvas, vector<Mat> frames) {
	/*
	GpuMat newFrame;
	GpuMat oriFrame(frame);
	cv::cuda::warpAffine(oriFrame, newFrame, mProjMat[vIdx], Size(frame.cols, frame.rows));
	*/
	for (int y=0; y<canvas.rows; y++) {
		for (int x=0; x<canvas.cols; x++) {
			int totalWeight = 0;
			Mat pixelMat = mProjMap[y][x];
			Vec3b pixel = Vec3b(0, 0, 0);
			// Get weight
			for (int v=0; v<mViewCount; v++) {
				if (mDebugView >= 0 && mDebugView != v)
					continue;
				if (pixelMat.at<int>(v, 0) != 0)
					totalWeight += 1;
			}

			for (int v=0; v<mViewCount; v++) {
				if (mDebugView >= 0 && mDebugView != v)
					continue;
				if (pixelMat.at<int>(v, 0) == 0)
					continue;
				int px = pixelMat.at<int>(v, 1) ;
				int py = pixelMat.at<int>(v, 2) ;
				pixel += (static_cast<float> (pixelMat.at<int>(v, 0)) / totalWeight) * frames[v].at<Vec3b>(py, px) ;
			}
			canvas.at<Vec3b>(y, x) = pixel;
		}
	}
}

MappingProjector::MappingProjector(int viewCount, Size viewSize, vector<struct MutualProjectParam> params, double focalLength) {
	mViewCount = viewCount;
	mViewSize = viewSize;
	mViewParams = params;
	mFocalLength = focalLength;
	mDebugView = -1;

	cout << "Focal Length: " << mFocalLength << endl;
	for (int v=0; v<mViewCount; v++)
		cout << "V: Theta: " << params[v].y << ", Phi: " << params[v].p << ", Omega: " << params[v].r << endl;

}

MappingProjector::~MappingProjector() {
	
}
