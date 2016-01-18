#include <header/MappingProjector.h>

void MappingProjector::calcProjectionMatrix(map< string, Mat > calibrationData) {
	calibrationData["cameraMatA"].convertTo(mA, CV_32F);
	mD = calibrationData["distCoeffs"];

	// Refresh focal length from pto file
	mA.at<float>(0, 0) = static_cast<float> ( mFocalLength );
	mA.at<float>(1, 1) = static_cast<float> ( mFocalLength );

	mSphericalWarper = shared_ptr<cv::detail::SphericalWarper>( new cv::detail::SphericalWarper(1.f) );
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
		Mat r = Mat::zeros(3, 3, CV_32F);
		struct MutualProjectParam vP = mViewParams[v];

		/*
		r.at<float>(0, 0) = cos(vP.y) * cos(vP.p);
		r.at<float>(0, 1) = cos(vP.y) * sin(vP.p) * sin(vP.r) - sin(vP.y) * cos(vP.r);
		r.at<float>(0, 2) = cos(vP.y) * sin(vP.p) * cos(vP.r) + sin(vP.y) * sin(vP.r);

		r.at<float>(1, 0) = sin(vP.y) * cos(vP.p);
		r.at<float>(1, 1) = sin(vP.y) * sin(vP.p) * sin(vP.r) + cos(vP.y) * cos(vP.r);
		r.at<float>(1, 2) = sin(vP.y) * sin(vP.p) * cos(vP.r) - cos(vP.y) * sin(vP.r);

		r.at<float>(2, 0) = -sin(vP.p);
		r.at<float>(2, 1) = cos(vP.p) * sin(vP.r);
		r.at<float>(2, 2) = cos(vP.p) * cos(vP.r);
		*/
		Mat rotationVec(1, 3, CV_32F);
		rotationVec.at<float>(0, 0) = vP.p;
		rotationVec.at<float>(0, 1) = vP.y;
		rotationVec.at<float>(0, 2) = vP.r;
		Rodrigues(rotationVec, r);

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
