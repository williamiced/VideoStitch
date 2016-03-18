#include "ManualFeatureGenerator.h"

void mouseEventCallback(int event, int x, int y, int flags, void* userdata) {
	if (!gBC->isWindowActive())
		return;
	if (event == EVENT_LBUTTONDOWN) 
		gBC->leftBtn(x, y);
    else if  ( event == EVENT_RBUTTONDOWN ) 
    	gBC->rightBtn(x, y);
    else if ( event == EVENT_MBUTTONDOWN ) 
    	gBC->middleBtn(x, y);
}

int ManualFeatureGenerator::loadImages(char* fileName) {
	ifstream inputFile(fileName);

	string imageName;
	while (getline(inputFile, imageName)) {
		Mat img;
		img = imread(imageName);
		mRawImages.push_back(img);
	}
	return mRawImages.size();
}

void ManualFeatureGenerator::runMarkProcess() {
	Size imgSize = Size(WINDOW_WIDTH/2, WINDOW_HEIGHT);
	//double scale = min(imgSize.width / WINDOW_WIDTH, imgSize.height / WINDOW_HEIGHT);
	for (int i=0; i<mImageCount; i++) {
		for (int j=i+1; j<mImageCount; j++) {
			mShowedImg = Mat(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC3);
			Mat resizedImg1, resizedImg2;
			resize(mRawImages[i], resizedImg1, imgSize );
			resize(mRawImages[j], resizedImg2, imgSize );
			resizedImg1.copyTo( mShowedImg(Rect(Point(0, 0), imgSize) ) );
			resizedImg2.copyTo( mShowedImg(Rect(Point(WINDOW_WIDTH/2, 0), imgSize)) );

			mViewIdx1 = i;
			mViewIdx2 = j;

			imshow(WINDOW_NAME, mShowedImg);
			clearBuffer();
			mIsWindowActive = true;
			waitKey(0);
			mIsWindowActive = false;
			writeToFile();
		}
	}
}

void ManualFeatureGenerator::clearBuffer() {
	mLeftBuffer.clear();
	mRightBuffer.clear();
	mColorBuffer.clear();
}

void ManualFeatureGenerator::writeToFile() {
	Size imgSize = mRawImages[0].size();
	double scaleX = imgSize.width / (WINDOW_WIDTH / 2);
	double scaleY = imgSize.height / (WINDOW_HEIGHT);

	ofstream outfile;

  	outfile.open(FILE_NAME, ios_base::app);
  	outfile << stringFormat("#%d-#%d\n", mViewIdx1, mViewIdx2);
  	for (int i=0; i<min(mLeftBuffer.size(), mRightBuffer.size()); i++) 
  		outfile << stringFormat("%d,%d@%d,%d\n", (int)(mLeftBuffer[i].x * scaleX), (int)(mLeftBuffer[i].y * scaleY), (int)(mRightBuffer[i].x * scaleX), (int)(mRightBuffer[i].y * scaleY));
}

bool ManualFeatureGenerator::isWindowActive() {
	return mIsWindowActive;
}

void ManualFeatureGenerator::leftBtn(int x, int y) {
	if (x < WINDOW_WIDTH/2) {
		mLeftBuffer.push_back(Point(x, y));
		mColorBuffer.push_back(genRandomScalar());
	} else {
		mRightBuffer.push_back(Point(x - WINDOW_WIDTH/2, y));
		if (mRightBuffer.size() > mColorBuffer.size())
			mColorBuffer.push_back(genRandomScalar());
	}
	redrawWindow();
}

void ManualFeatureGenerator::rightBtn(int x, int y) {
	if (x < WINDOW_WIDTH/2) {
		mLeftBuffer.pop_back();
	} else {
		mRightBuffer.pop_back();
	}
	redrawWindow();
}

void ManualFeatureGenerator::middleBtn(int x, int y) {

}

void ManualFeatureGenerator::redrawWindow() {
	Mat newShowedImg = mShowedImg.clone();
	for (int i=0; i<mLeftBuffer.size(); i++) 
		circle(newShowedImg, mLeftBuffer[i], 5, mColorBuffer[i], -1);
	for (int i=0; i<mRightBuffer.size(); i++) 
		circle(newShowedImg, mRightBuffer[i] + Point(WINDOW_WIDTH/2, 0), 5, mColorBuffer[i], -1);

	for (int i=0; i<min(mLeftBuffer.size(), mRightBuffer.size()); i++) 
		line(newShowedImg, mLeftBuffer[i], mRightBuffer[i] + Point(WINDOW_WIDTH/2, 0), Scalar(0), 2);

	imshow(WINDOW_NAME, newShowedImg);
	waitKey(0);
}

Scalar ManualFeatureGenerator::genRandomScalar() {
	return Scalar(rand() % 255, rand() % 255, rand() % 255);
}

ManualFeatureGenerator::ManualFeatureGenerator(char* fileName) {
	mIsWindowActive = false;
	mImageCount = loadImages(fileName);
	if (mImageCount < 2)
		logMsg(LOG_ERROR, stringFormat("Too few input images: %d", mImageCount ));
}

int main(int argc, char** argv) {
	if ( !checkArguments(argc, argv) )
		exitWithMsg(E_BAD_ARGUMENTS);

	srand(time(NULL));
	gBC = unique_ptr<ManualFeatureGenerator>(new ManualFeatureGenerator( getCmdOption(argv, argv + argc, "--input") ) );

	namedWindow(WINDOW_NAME, 1);
	setMouseCallback(WINDOW_NAME, mouseEventCallback, NULL);
	
	gBC->runMarkProcess();
}