#include <header/MappingProjector.h>

void MappingProjector::calcProjectionMatrix(map< string, Mat > calibrationData) {
	mA = calibrationData["cameraMatA"];
	mD = calibrationData["distCoeffs"];

	mFocalLength = sqrt( pow(mA.at<double>(0, 0), 2) + pow(mA.at<double>(1, 1), 2)  );

	for (int y=0; y<OUTPUT_PANO_HEIGHT; y++) {
		vector<Mat> weightMatRow;
		for (int x=0; x<OUTPUT_PANO_WIDTH; x++) {
			double theta = x * 2 * M_PI / OUTPUT_PANO_WIDTH;
			double phi = y * M_PI / OUTPUT_PANO_HEIGHT;

			Mat weightMat = calcWeightForEachView(theta, phi);
			weightMatRow.push_back(weightMat);
		}
		mProjMat.push_back(weightMatRow);
	}
}

Mat MappingProjector::calcWeightForEachView(double theta, double phi) {
	Mat wM(mViewCount, 3, CV_32S);

	int xc = mViewSize.width/2;
	int yc = mViewSize.height/2;
	for (int v=0; v<mViewCount; v++) {
		if ( !isViewCloseEnough(theta, phi, v) ) {
			wM.at<int>(v, 0) = 0;
			wM.at<int>(v, 1) = 0;
			wM.at<int>(v, 2) = 0;
		} else {
			// Uniform blending
			wM.at<int>(v, 0) = 1;

			double thetaDiff = theta;
			double phiDiff = phi;
			getAngleDiff(thetaDiff, phiDiff, v);

			int x = (int) mFocalLength * tan(thetaDiff);
			int y = (int) mFocalLength * tan(phiDiff) / cos(thetaDiff);
			int xd = (x + xc) < 0 ? 0 : ( (x + xc) < (mViewSize.width - 1) ? (x + xc) : (mViewSize.width - 1) ) ;
			int yd = (y + yc) < 0 ? 0 : ( (y + yc) < (mViewSize.height - 1) ? (y + yc) : (mViewSize.height - 1) ) ;
			
			if (xd != (x + xc) || yd != (y+yc)) 
				wM.at<int>(v, 0) = 0;
			else {
				wM.at<int>(v, 1) = xd;
				wM.at<int>(v, 2) = yd;
			}
		}
	}
	return wM;
}

void MappingProjector::getAngleDiff(double& theta, double& phi, int vIdx) {
	bool isCrossTheta = false;
	bool isCrossPhi = false;

	double thetaDist = fabs(theta - mViewCenter[vIdx].x);
	if (thetaDist > M_PI) {
		thetaDist = 2*M_PI - thetaDist;
		isCrossTheta = true;
	}
	theta = thetaDist * ((( !isCrossTheta && theta < mViewCenter[vIdx].x ) || ( isCrossTheta && theta >= mViewCenter[vIdx].x )) ? (-1) : 1);
	phi = phi - mViewCenter[vIdx].y;	
}

bool MappingProjector::isViewCloseEnough(double theta, double phi, int vIdx) {
	double thetaAbs = theta;
	double phiAbs = phi;
	getAngleDiff(thetaAbs, phiAbs, vIdx);
	thetaAbs = fabs(thetaAbs);
	phiAbs = fabs(phiAbs);
	double deltaSigmaStep1 = sqrt( pow(sin(thetaAbs/2), 2) + cos(theta) * cos(mViewCenter[vIdx].x) * pow(sin(phiAbs/2), 2) );
	double deltaSigmaStep2 = 2 * asin( deltaSigmaStep1 );
	double greatCircleDist = PROJECT_CIRCLE_LEN * deltaSigmaStep2;

	return greatCircleDist < 1;
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
			Mat pixelMat = mProjMat[y][x];
			Vec3b pixel = Vec3b(0, 0, 0);
			for (int v=0; v<mViewCount; v++)
				if (pixelMat.at<int>(v, 0) != 0)
					totalWeight += 1;
			/*
			if (totalWeight > 0)
				canvas.at<Vec3b>(y, x) = Vec3b(255, 255, 255);
			else
				canvas.at<Vec3b>(y, x) = Vec3b(0, 0, 0);
			continue;
			*/
			for (int v=0; v<mViewCount; v++) {
				if (pixelMat.at<int>(v, 0) == 0)
					continue;
				/*
				if (v == 0)
					pixel += Vec3b(50, 0, 0);
				else if (v == 1)
					pixel += Vec3b(100, 0, 0);
				else if (v == 2)
					pixel += Vec3b(0, 50, 0);
				else if (v == 3)
					pixel += Vec3b(0, 100, 0);
				else if (v == 4)
					pixel += Vec3b(0, 0, 50);
				else if (v == 5)
					pixel += Vec3b(0, 0, 100);
				*/
				int px = pixelMat.at<int>(v, 1) ;
				int py = pixelMat.at<int>(v, 2) ;
				//cout << "v1: " << pixelMat.at<int>(v, 1) << ", v2: " << pixelMat.at<int>(v, 2) << endl;
				pixel += pixelMat.at<int>(v, 0) * frames[v].at<Vec3b>(py, px) / totalWeight;
			}
			//if (totalWeight > 0) // avoid floating number problem
			//	pixel /= totalWeight;
			//cout << "Color: " << (int)pixel(0) << ", " << (int)pixel(1) << ", " << (int)pixel(2) << endl;
			canvas.at<Vec3b>(y, x) = pixel;

		}
	}


	
	// [TODO] Currently copyTo is a bottleneck which reduce fps from 40 to 13
	// newFrame.copyTo( canvas );
	/** 
		[TODO]
			Paste new frame onto canvas
	*/
}

MappingProjector::MappingProjector(int viewCount, Size viewSize) {
	mViewCount = viewCount;
	mViewSize = viewSize;

	// [TODO] Not yet generated
	mViewCenter.push_back( cv::Point2d(0.f, M_PI/2) );
	mViewCenter.push_back( cv::Point2d(M_PI/2, M_PI/2) );
	mViewCenter.push_back( cv::Point2d(M_PI, M_PI/2) );
	mViewCenter.push_back( cv::Point2d(M_PI*3/2, M_PI/2) );
	mViewCenter.push_back( cv::Point2d(0.f, 0.f) );
	mViewCenter.push_back( cv::Point2d(0.f, M_PI) );
}

MappingProjector::~MappingProjector() {
	
}