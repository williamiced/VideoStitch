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

	Size imgSize = mRawImages[0].size();
	double scaleX = (double)(WINDOW_WIDTH / 2) / imgSize.width;
	double scaleY = (double)(WINDOW_HEIGHT / 1) / imgSize.height;

	if (mMatchInfos.size() != 0) {
		for (int i=0; i<mMatchInfos.size(); i++) {
			if (mViewIdx1 == mMatchInfos[i].idx1 && mViewIdx2 == mMatchInfos[i].idx2) {
				for (int m=0; m<mMatchInfos[i].matches.size(); m++) {
					mLeftBuffer.push_back( Point(mMatchInfos[i].matches[m].p1.x * scaleX, mMatchInfos[i].matches[m].p1.y * scaleY) );
					mRightBuffer.push_back( Point(mMatchInfos[i].matches[m].p2.x * scaleX, mMatchInfos[i].matches[m].p2.y * scaleY) );
					mColorBuffer.push_back(genRandomScalar());
				}
				break;
			} else if (mViewIdx1 == mMatchInfos[i].idx2 && mViewIdx2 == mMatchInfos[i].idx1) {
				for (int m=0; m<mMatchInfos[i].matches.size(); m++) {
					mLeftBuffer.push_back( Point(mMatchInfos[i].matches[m].p2.x * scaleX, mMatchInfos[i].matches[m].p2.y * scaleY) );
					mRightBuffer.push_back( Point(mMatchInfos[i].matches[m].p1.x * scaleX, mMatchInfos[i].matches[m].p1.y * scaleY) );
					mColorBuffer.push_back(genRandomScalar());
				}
				break;
			}
		}
		redrawWindow();
	}
}

void ManualFeatureGenerator::writeToFile() {
	Size imgSize = mRawImages[0].size();
	double scaleX = (double)imgSize.width / (WINDOW_WIDTH / 2);
	double scaleY = (double)imgSize.height / (WINDOW_HEIGHT);

	cout << "scaleX: " << scaleX << endl;
	cout << "scaleY: " << scaleY << endl;

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

void ManualFeatureGenerator::loadFeatureInfoFromFile(char* fileName) {
	logMsg(LOG_INFO, "Load features file");	
	ifstream inputFile(fileName);

	string str;
	int idx1 = -1;
	int idx2 = -1;
	vector<FeatureMatch> currentFM;
	while (getline(inputFile, str)) {
		if (str.find("#") == 0) {
			if (idx1 >= 0) {
				MatchInfo mi;
				mi.idx1 = idx1;
				mi.idx2 = idx2;
				mi.matches = currentFM;
				mMatchInfos.push_back(mi);
				currentFM.clear();
			}
			idx1 = str.at(1) - '0';
			idx2 = str.at(4) - '0';
		} else {
			vector<int> tmpVec;
			char * cstr = new char [str.length()+1];
  			std::strcpy (cstr, str.c_str());

  			char * p = std::strtok (cstr,",@");
			while (p!=0) {
				tmpVec.push_back(atoi(p));
			    p = std::strtok(NULL,",@");
			}

			delete[] cstr;

			FeatureMatch fm;
			fm.p1 = Point(tmpVec[0], tmpVec[1]);
			fm.p2 = Point(tmpVec[2], tmpVec[3]);
			currentFM.push_back(fm);
		}
	}
	MatchInfo mi;
	mi.idx1 = idx1;
	mi.idx2 = idx2;
	mi.matches = currentFM;
	mMatchInfos.push_back(mi);
	currentFM.clear();
}

int main(int argc, char** argv) {
	if ( !checkArguments(argc, argv) )
		exitWithMsg(E_BAD_ARGUMENTS);

	srand(time(NULL));
	gBC = unique_ptr<ManualFeatureGenerator>(new ManualFeatureGenerator( getCmdOption(argv, argv + argc, "--input") ) );

	namedWindow(WINDOW_NAME, 1);
	setMouseCallback(WINDOW_NAME, mouseEventCallback, NULL);
	
	if ( getCmdOption(argv, argv + argc, "--featureInfo") != 0 ) {
		gBC->loadFeatureInfoFromFile( getCmdOption(argv, argv + argc, "--featureInfo") );
	}

	gBC->runMarkProcess();
}