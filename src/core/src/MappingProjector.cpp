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

void MappingProjector::calcProjectionMatrix() {
	// Initialize warpers
	setupWarpers();

	// Build maps
	buildMapsForViews();

	// Calculate the size of current canvas
	updateCurrentCanvasROI();
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

void MappingProjector::renderInterestArea(Mat& outImg, vector<Mat> frames, float u1, float u2, float v1, float v2) {
	/** 
		Render area from tl(u1, v1) to br(u2, v2) 
		Using inverseMap
		u : [-PI, PI]
		v : [0, PI]
	*/
	logMsg(LOG_DEBUG, stringFormat("Render from theta(%f -> %f), phi(%f -> %f)", u1, u2, v1, v2) );
	unsigned int canvasWidth = mCanvasROI.size().width;
	unsigned int canvasHeight = mCanvasROI.size().height;
	unsigned int tlx = (u1 + M_PI) * canvasWidth / (2 * M_PI);
	unsigned int tly = v1 * canvasHeight / M_PI;
	unsigned int brx = (u2 + M_PI) * canvasWidth / (2 * M_PI);
	unsigned int bry = v2 * canvasHeight / M_PI;
	unsigned int outWidth 	= (brx > tlx ? (brx-tlx) : (brx-tlx + canvasWidth) );
	unsigned int outHeight 	= (bry > tly ? (bry-tly) : (bry-tly + canvasHeight) );

	outImg = Mat(mOutputWindowSize.height, mOutputWindowSize.width, CV_8UC3);

	float scaleX = static_cast<float>(mOutputWindowSize.width) / outWidth;
	float scaleY = static_cast<float>(mOutputWindowSize.height) / outHeight;

	for (int y = 0; y < mOutputWindowSize.height; y++) {
		for (int x = 0; x < mOutputWindowSize.width; x++) {
			int canvasX = static_cast<int>( x / scaleX + tlx ) % canvasWidth;
			int canvasY = static_cast<int>( y / scaleY + tly ) % canvasHeight;
			outImg.at<Vec3b>(y, x) = getInversePixel(canvasY, canvasX, frames);
		}
	}
}

Vec3b MappingProjector::getInversePixel(int y, int x, vector<Mat> frames) {
	Vec3b pixelValue = Vec3b(0, 0, 0);
	for (int v=0; v<mViewCount; v++) {
		if ( !mMapROIs[v].contains( Point(x, y) ) )
			continue;
		int vx = static_cast<int> (mUxMaps[v].at<float>(y - mMapROIs[v].tl().y, x - mMapROIs[v].tl().x) );
		int vy = static_cast<int> (mUyMaps[v].at<float>(y - mMapROIs[v].tl().y, x - mMapROIs[v].tl().x) );
		// Not blend yet
		if (vy >= mViewSize.height || vx >= mViewSize.width || vy < 0 || vx < 0)
			continue;
		pixelValue = frames[v].at<Vec3b>(vy, vx);
	}
	return pixelValue;
}

void MappingProjector::defineWindowSize() {
	mOutputWindowSize = Size(OUTPUT_WINDOW_WIDTH, OUTPUT_WINDOW_HEIGHT);
}

Size MappingProjector::getOutputImageSize() {
	return mOutputWindowSize;
}

MappingProjector::MappingProjector(int viewCount, Size viewSize) : 
	mFrameProcessed(0),
	mViewCount(viewCount),
	mViewSize(viewSize) {
		defineWindowSize();
}