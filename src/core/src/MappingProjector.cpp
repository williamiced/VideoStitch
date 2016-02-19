#include <header/MappingProjector.h>

void MappingProjector::setCameraParams(vector<struct MutualProjectParam> params, double focalLength) {
	// Calculate R by eular angles
	for (int v=0; v<mViewCount; v++) {
		Mat r = Mat::zeros(3, 3, CV_32F);
		struct MutualProjectParam vP = params[v];
		double alpha = vP.y;
		double beta = vP.p;
		double gamma = vP.r;

		// Take camera as reference coordinate system, around: x-axis -> pitch, y-axis -> yaw, z->axis -> roll
		Mat Rz = getZMatrix(gamma);
		Mat Ry = getYMatrix(alpha);
		Mat Rx = getXMatrix(beta);
		r = Ry * Rx * Rz;
		mR.push_back( r );
	}

	// Calculate mK
	mK.resize(mViewCount);
	for (int v=0; v<mViewCount; v++) {
		mK[v] = Mat::zeros(3, 3, CV_32F);
		mK[v].at<float>(0, 0) = static_cast<float> ( focalLength );
		mK[v].at<float>(1, 1) = static_cast<float> ( focalLength );
		mK[v].at<float>(0, 2) = static_cast<float> ( mViewSize.width/2 );
		mK[v].at<float>(1, 2) = static_cast<float> ( mViewSize.height/2 );
		mK[v].at<float>(2, 2) = 1.f;
	}
}

void MappingProjector::setCameraParams(vector<Mat> Rs, vector<Mat> Ks) {
	if ( !Rs.empty() )
		mR = Rs;
	if ( !Ks.empty() ) 
		mK = Ks;
}

Size MappingProjector::calcProjectionMatrix() {
	// Initialize warpers
	setupWarpers();

	// Build maps
	buildMapsForViews();

	// Calculate the size of current canvas
	updateCurrentCanvasROI();

	// Construct inverse map by maps
	constructInverseMaps();

	return mCanvasROI.size();
}

void MappingProjector::setupWarpers() {
	for (int v=0; v<mViewCount; v++) {
		// Use focal length as scale
		float scale = (mK[v].at<float>(0, 0) + mK[v].at<float>(1, 1))/2 ;
		logMsg(LOG_DEBUG, stringFormat("Project scale: %f", scale) );
		mSphericalWarpers.push_back( shared_ptr<PROJECT_METHOD>( new PROJECT_METHOD( scale ) ) );
	}
}

void MappingProjector::buildMapsForViews() {
	for (int v=0; v<mViewCount; v++) {
		Mat uxmap, uymap;
		mMapROIs.push_back( mSphericalWarpers[v]->buildMaps(mViewSize, mK[v], mR[v], uxmap, uymap) );
		mUxMaps.push_back( uxmap );
		mUyMaps.push_back( uymap );
		cout << "Ux map size: " << uxmap.rows << ", " << uxmap.cols << endl;
		cout << "Uy map size: " << uymap.rows << ", " << uymap.cols << endl;
	}
}

void MappingProjector::updateCurrentCanvasROI() {
	// Calculate canvas ROI
	mCanvasROI = mMapROIs[0];
	for (int v=1; v<mViewCount; v++)
		mCanvasROI = mCanvasROI | mMapROIs[v];
	mCanvasROI += Size(1, 1);

	for (int v=0; v<mViewCount; v++) 
		mMapROIs[v] = Rect( mMapROIs[v].x - mCanvasROI.x, mMapROIs[v].y - mCanvasROI.y, mMapROIs[v].width+1, mMapROIs[v].height+1 );
}

void MappingProjector::constructInverseMaps() {
	for (int v=0; v<mViewCount; v++) {
		mInverseMapX.push_back( Mat::zeros(mFinalCanvasSize.height, mFinalCanvasSize.width, CV_32SC1) );
		mInverseMapY.push_back( Mat::zeros(mFinalCanvasSize.height, mFinalCanvasSize.width, CV_32SC1) );

		for(int y = 0; y < mViewSize.height; y++) {
			for(int x = 0; x < mViewSize.width; x++) {
				int xInCanvas = static_cast<int>( (mUxMaps[v].at<float>(y, x) + mMapROIs[v].tl().x) * mFinalCanvasSize.width / mCanvasROI.size().width );
				int yInCanvas = static_cast<int>( (mUyMaps[v].at<float>(y, x) + mMapROIs[v].tl().y) * mFinalCanvasSize.height / mCanvasROI.size().height );

				//cout << "oy: " << yInCanvas << ", ox: " << xInCanvas << ", x:" << x << ", y:" << y << endl;
				if (xInCanvas >= mFinalCanvasSize.width || yInCanvas >= mFinalCanvasSize.height)
					continue;

				mInverseMapX[v].at<int>( yInCanvas, xInCanvas ) = x;
				mInverseMapY[v].at<int>( yInCanvas, xInCanvas ) = y;
				
			}
		}
	}
}

void MappingProjector::renderInterestArea(Mat& outImg, vector<Mat> frames, float u1, float u2, float v1, float v2) {
	/** 
		Render area from tl(u1, v1) to br(u2, v2) 
		Using inverseMap
		u : [-PI, PI]
		v : [0, PI]
	*/
	unsigned int canvasWidth = mFinalCanvasSize.width;
	unsigned int canvasHeight = mFinalCanvasSize.height;
	unsigned int tlx = (u1 + M_PI) * canvasWidth / (2 * M_PI);
	unsigned int tly = v1 * canvasHeight / M_PI;
	unsigned int brx = (u2 + M_PI) * canvasWidth / (2 * M_PI);
	unsigned int bry = v2 * canvasHeight / M_PI;
	unsigned int outWidth 	= (brx > tlx ? (brx-tlx) : (brx-tlx + canvasWidth) );
	unsigned int outHeight 	= (bry > tly ? (bry-tly) : (bry-tly + canvasHeight) );

	outImg = Mat(outHeight, outWidth, CV_8UC3);
	
	for (unsigned int y = 0; y < outHeight; y++ ) 
		for (unsigned int x = 0; x < outWidth; x++ ) 
			outImg.at<Vec3b>(y, x) = getInversePixel( (tly+y)%canvasHeight, (tlx+x)%canvasWidth, frames);
}

Vec3b MappingProjector::getInversePixel(unsigned int y, unsigned int x, vector<Mat> frames) {
	cout << y << ", " << x << endl;
	Vec3b pixelValue = Vec3b(0, 0, 0);
	for (int v=0; v<mViewCount; v++) {
		unsigned int vx = mInverseMapX[v].at<int>(y, x);
		unsigned int vy = mInverseMapY[v].at<int>(y, x);
		cout << vy << " ??? " << vx << endl;

		if (vx == 0 && vy == 0)
			continue;
		// Not blend yet
		pixelValue = frames[v].at<Vec3b>(vy, vx);
	}
	return pixelValue;
}

void MappingProjector::defineCanvasSize() {
	mFinalCanvasSize = Size(1000, 500);
}

MappingProjector::MappingProjector(int viewCount, Size viewSize) : 
	mViewCount(viewCount),
	mViewSize(viewSize),
	mFrameProcessed(0) {
		defineCanvasSize();
}