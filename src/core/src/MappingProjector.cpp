#include <header/MappingProjector.h>

void MappingProjector::setCameraParams(vector<struct MutualProjectParam> params, double focalLength) {
	// Calculate R by eular angles
	for (int v=0; v<mViewCount; v++) {
		Mat r = Mat::zeros(3, 3, CV_32F);
		struct MutualProjectParam vP = params[v];
		r = getRotationMatrix(vP.y, vP.p, vP.r);
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

bool MappingProjector::checkSeriailFileExist(string filename) {
	struct stat buffer;   
  	return (stat (filename.c_str(), &buffer) == 0); 
}

void MappingProjector::loadSerialFile() {
	mProjMasks.resize(mViewCount);
	mProjMapX.resize(mViewCount);
	mProjMapY.resize(mViewCount);
	for (int i=0; i<mViewCount; i++) {
		ifstream ifs(stringFormat("tmp/mask_%d.srl", i));
        boost::archive::binary_iarchive ia(ifs);
        ia >> mProjMasks[i];

        ifstream ifs2(stringFormat("tmp/projMapX_%d.srl", i));
        boost::archive::binary_iarchive ia2(ifs2);
        ia2 >> mProjMapX[i];

        ifstream ifs3(stringFormat("tmp/projMapY_%d.srl", i));
        boost::archive::binary_iarchive ia3(ifs3);
        ia3 >> mProjMapY[i];
	}
	logMsg(LOG_INFO, "=== Load serial files from disk ===");
}

void MappingProjector::saveSerialFile() {
	for (int i=0; i<mViewCount; i++) {
		ofstream ofs(stringFormat("tmp/mask_%d.srl", i));
        boost::archive::binary_oarchive oa(ofs);
        oa << mProjMasks[i];

		ofstream ofs2(stringFormat("tmp/projMapX_%d.srl", i));
        boost::archive::binary_oarchive oa2(ofs2);
        oa2 << mProjMapX[i];        

        ofstream ofs3(stringFormat("tmp/projMapY_%d.srl", i));
        boost::archive::binary_oarchive oa3(ofs3);
        oa3 << mProjMapY[i];        
	}	
	logMsg(LOG_INFO, "=== Save serial files to disk ===");
}

void MappingProjector::calcProjectionMatrix() {
#ifdef USING_SERIALIZABLE_RESULT
	if ( checkSeriailFileExist("tmp/mask_0.srl") ) {
		loadSerialFile();
	} else {
#endif
		// Initialize warpers
		setupWarpers();
		logMsg(LOG_INFO, "=== Constructing UV checkup table ===");
		constructUVcheckupTable();
		logMsg(LOG_INFO, "=== Interpolate UV checkup table ===");
		interpolateUVcheckupTable();
		logMsg(LOG_INFO, "=== Done projection matrix calculation ===");

#ifdef USE_HOMOGRAPHY_WARP	
		refineCheckupTableByFeaturesMatching();
		logMsg(LOG_INFO, "=== Refine complete ===");
#endif	

#ifdef USING_SERIALIZABLE_RESULT
		saveSerialFile();
	} 
#endif
	mBP->genWeightMapByMasks(mProjMasks);
	mBP->newPreprocess();
}

void MappingProjector::setupWarpers() {
	for (int v=0; v<mViewCount; v++) 
		mSphericalWarpers.push_back( shared_ptr<PROJECT_METHOD>( new PROJECT_METHOD( 1.f ) ) );
}

void MappingProjector::constructUVcheckupTable() {
	/** Initialize */
	mProjMasks.resize(mViewCount);
	mProjMapX.resize(mViewCount);
	mProjMapY.resize(mViewCount);
	for (int v=0; v<mViewCount; v++) {
		mProjMasks[v] = Mat::zeros(OUTPUT_PANO_HEIGHT, OUTPUT_PANO_WIDTH, CV_8UC1);
		mProjMapX[v] = Mat::zeros(OUTPUT_PANO_HEIGHT, OUTPUT_PANO_WIDTH, CV_32SC1);
		mProjMapY[v] = Mat::zeros(OUTPUT_PANO_HEIGHT, OUTPUT_PANO_WIDTH, CV_32SC1);
	}

	omp_lock_t writelock;
	omp_init_lock(&writelock);

	#pragma omp parallel for collapse(3)
	for (int y=0; y<mViewSize.height; y++) {
		for (int x=0; x<mViewSize.width; x++) {
			for (int v=0; v<mViewCount; v++) {
				Point2f mapPnt = mSphericalWarpers[v]->warpPoint( Point2f(x, y), mK[v], mR[v] );
				// mapPnt: x = (-pi ~ pi), y = (0 ~ pi)
				int oX = static_cast<int> ( (mapPnt.x - (-M_PI)) * OUTPUT_PANO_WIDTH / (2 * M_PI) ) % OUTPUT_PANO_WIDTH;
				int oY = static_cast<int> ( mapPnt.y * OUTPUT_PANO_HEIGHT / (M_PI) )  % OUTPUT_PANO_HEIGHT;

				omp_set_lock(&writelock);
				mProjMasks[v].at<uchar>(oY, oX) = 255;
				mProjMapX[v].at<int>(oY, oX) = x;
				mProjMapY[v].at<int>(oY, oX) = y;
				omp_unset_lock(&writelock);
			}
		}
	}
}

void MappingProjector::interpolateUVcheckupTable() {
	for (int y=0; y<OUTPUT_PANO_HEIGHT; y++) {
		for (int x=0; x<OUTPUT_PANO_WIDTH; x++) {
			bool hasPixel = false;
			for (int v=0; v<mViewCount; v++) {
				if (mProjMasks[v].at<uchar>(y, x) != 0)
					hasPixel = true;
			}
			if (!hasPixel) {
				for (int v=0; v<mViewCount; v++) {
					if (mProjMasks[v].at<uchar>(y, x) != 0)
						continue;
					if (x > 0 && mProjMasks[v].at<uchar>(y, x-1) != 0) {
						mProjMapX[v].at<int>(y, x) = mProjMapX[v].at<int>(y, x-1);
						mProjMapY[v].at<int>(y, x) = mProjMapY[v].at<int>(y, x-1);
						mProjMasks[v].at<uchar>(y, x) = 255;
					} else if (y > 0 && mProjMasks[v].at<uchar>(y-1, x) != 0) {
						mProjMapX[v].at<int>(y, x) = mProjMapX[v].at<int>(y-1, x);
						mProjMapY[v].at<int>(y, x) = mProjMapY[v].at<int>(y-1, x);
						mProjMasks[v].at<uchar>(y, x) = 255;
					}
				} 
			}
		}
	}

	for (int y=OUTPUT_PANO_HEIGHT-1; y>=0; y--) {
		for (int x=OUTPUT_PANO_WIDTH-1; x>=0; x--) {
			bool hasPixel = false;
			for (int v=0; v<mViewCount; v++) {
				if (mProjMasks[v].at<uchar>(y, x) != 0)
					hasPixel = true;
			}
			if (!hasPixel) {
				for (int v=0; v<mViewCount; v++) {
					if (mProjMasks[v].at<uchar>(y, x) != 0)
						continue;
					if (x < OUTPUT_PANO_WIDTH-1 && mProjMasks[v].at<uchar>(y, x+1) != 0) {
						mProjMapX[v].at<int>(y, x) = mProjMapX[v].at<int>(y, x+1);
						mProjMapY[v].at<int>(y, x) = mProjMapY[v].at<int>(y, x+1);
						mProjMasks[v].at<uchar>(y, x) = 255;
					} else if (y < OUTPUT_PANO_HEIGHT-1 && mProjMasks[v].at<uchar>(y+1, x) != 0) {
						mProjMapX[v].at<int>(y, x) = mProjMapX[v].at<int>(y+1, x);
						mProjMapY[v].at<int>(y, x) = mProjMapY[v].at<int>(y+1, x);
						mProjMasks[v].at<uchar>(y, x) = 255;
					}
				} 
			}
		}
	}
}

void MappingProjector::refineCheckupTableByFeaturesMatching() {
	// For every two views, warp the first view to match second view
	for (int v=0; v<mViewCount; v++) {
		vector<Mat> Hs;
		mHs.push_back(Hs);
	}

	for (size_t i=0; i<mMatchInfos.size(); i++) {
		int idx1 = mMatchInfos[i].idx1;
		int idx2 = mMatchInfos[i].idx2;

		//if ( !( (idx1 == 0 && idx2 == 5) || (idx1 == 5 && idx2 == 0) ))
		//	continue;

		vector<Point2d> matchesInIdx1;
		vector<Point2d> matchesInIdx2;

		vector<FeatureMatch> matches = mMatchInfos[i].matches;
		if (matches.size() < 9)
			continue;
		for (size_t m=0; m<matches.size(); m++) {
			Point2f p1 = mSphericalWarpers[idx1]->warpPoint( Point2f(matches[m].p1.x, matches[m].p1.y), mK[idx1], mR[idx1] );
			Point2f p2 = mSphericalWarpers[idx2]->warpPoint( Point2f(matches[m].p2.x, matches[m].p2.y), mK[idx2], mR[idx2] );

			int oX = static_cast<int> ( (p1.x - (-M_PI)) * OUTPUT_PANO_WIDTH / (2 * M_PI) ) % OUTPUT_PANO_WIDTH;
			int oY = static_cast<int> ( p1.y * OUTPUT_PANO_HEIGHT / (M_PI) )  % OUTPUT_PANO_HEIGHT;
			Point2d p1d = Point2d( (double)oX, (double)oY );

			oX = static_cast<int> ( (p2.x - (-M_PI)) * OUTPUT_PANO_WIDTH / (2 * M_PI) ) % OUTPUT_PANO_WIDTH;
			oY = static_cast<int> ( p2.y * OUTPUT_PANO_HEIGHT / (M_PI) )  % OUTPUT_PANO_HEIGHT;
			Point2d p2d = Point2d( (double)oX, (double)oY );

			matchesInIdx1.push_back( p1d );
			matchesInIdx2.push_back( p2d );
			//matchesInIdx2.push_back( p1d + Point2d(40, 40) );
		}
		vector<unsigned char> inliersMask(matchesInIdx1.size());

		Mat H = findHomography(matchesInIdx1, matchesInIdx2, CV_RANSAC, RANSAC_REPROJ_THRES, inliersMask);
		cout << H << endl;

		for (int j=0; j<(int) inliersMask.size(); j++) {
			cout << (int)inliersMask[j] << " ";
		}

		cout << stringFormat("view: %d->%d, matches: %d", idx1, idx2, matches.size()) << endl;
		
		mHs[idx1].push_back(H);
		/*
		Mat tmpMap;
		mProjMapX[idx1].convertTo(tmpMap, CV_32F);
		cv::warpPerspective(tmpMap, tmpMap, H, mProjMapX[idx1].size());
		tmpMap.convertTo(mProjMapX[idx1], CV_8U);

		Mat tmpMap2;
		mProjMapY[idx1].convertTo(tmpMap2, CV_32F);
		cv::warpPerspective(tmpMap2, tmpMap2, H, mProjMapY[idx1].size());
		tmpMap2.convertTo(mProjMapY[idx1], CV_8U);
		*/
		// Can have some issues here, not sure
	}
}

void MappingProjector::tuneToMap(Point2f& p) {
	while (p.x < -M_PI)
		p.x += 2*M_PI;
	while (p.x > M_PI)
		p.x -= 2*M_PI;
	while (p.y < 0)
		p.y += M_PI;
	while (p.y > M_PI) 
		p.y -= M_PI;
}

void MappingProjector::getUVbyAzimuthal(const float xOffset, const float yOffset, const Point2f center, Point2f& newPnt) {
	float phi0 = center.y - M_PI/2.f;
	float lambda0 = center.x + M_PI;

	if (xOffset == 0.f && yOffset == 0.f) {
		newPnt.x = center.x;
		newPnt.y = center.y;
		return;
	}

	float c = sqrt(xOffset * xOffset + yOffset * yOffset);
	float cosc = cos(c);
	float sinc = sqrt(1-cosc*cosc);
	float sinPhi0 = sin(phi0);
	float cosPhi0 = sqrt(1-sinPhi0*sinPhi0);
	float phi = asin( cosc * sinPhi0 + yOffset * sinc * cosPhi0 / c);
	float lambda;

	if (phi0 == M_PI/2)
		lambda = lambda0 + atan2(-yOffset, xOffset);
	else if (phi0 == -M_PI)
		lambda = lambda0 + atan2(yOffset, xOffset);
	else
		lambda = lambda0 + atan2( xOffset * sinc, c * cosPhi0 * cosc - yOffset * sinPhi0 * sinc );

	newPnt.x = lambda - M_PI;
	newPnt.y = phi + M_PI/2.f;
}

void MappingProjector::initialData() {
	int h, w;
	w = OUTPUT_PANO_WIDTH;
	h = OUTPUT_PANO_HEIGHT;
	mWarpedImgs.resize(mViewCount);
	for (int v=0; v<mViewCount; v++)
		mWarpedImgs[v] = Mat::zeros(h, w, CV_8UC3);
	
	vector<Point> corners;
	for (int v=0; v<mViewCount; v++)
		corners.push_back(Point(0, 0));

	mBP = shared_ptr<BlendingProcessor>( new BlendingProcessor( mViewCount, Rect(0, 0, w, h) ) );
	mEP = shared_ptr<ExposureProcessor>( new ExposureProcessor( corners, mViewCount) );
}

Size MappingProjector::getOutputVideoSize() {
	return Size(OUTPUT_PANO_WIDTH, OUTPUT_PANO_HEIGHT);
}

void MappingProjector::increaseFrame() {
	mFrameProcessed++;
}

void MappingProjector::checkFPS() {
	double total = 0.f;
	for (unsigned int i=0; i<mExecTimes.size(); i++) 
		total += mExecTimes[i];
	logMsg(LOG_INFO, stringFormat( "=== Average FPS is %lf === ", mFrameProcessed / total ) );
}

MappingProjector::MappingProjector(int viewCount, Size viewSize) : 
	mFrameProcessed(0),
	mViewCount(viewCount),
	mViewSize(viewSize) {
		initialData();
}

void MappingProjector::genExpoBlendingMap(vector<Mat> frames) {
	for (int v=0; v<mViewCount; v++)
		mWarpedImgs[v] = Mat::zeros(OUTPUT_PANO_HEIGHT, OUTPUT_PANO_WIDTH, CV_8UC3);

	#pragma omp parallel for collapse(2)
	for (int y=0; y<OUTPUT_PANO_HEIGHT; y++) {
		for (int x=0; x<OUTPUT_PANO_WIDTH; x++) {
			for (int v=0; v<mViewCount; v++) {
				if (mProjMasks[v].at<uchar>(y, x) != 0) {
					int px = mProjMapX[v].at<int>(y, x);
					int py = mProjMapY[v].at<int>(y, x);
					if (py < 0 || px < 0 || px >= mViewSize.width || py >= mViewSize.height)
						mWarpedImgs[v].at<Vec3b>(y, x) = Vec3b(0, 0, 0);	
					else
						mWarpedImgs[v].at<Vec3b>(y, x) = frames[v].at<Vec3b>(py, px);
				}
			}
		}
	}
	
	mEP->feedExposures(mWarpedImgs, mProjMasks);
	mBP->genFinalMap(mEP->gains());
	mBP->getFinalMap(mFinalBlendingMap);
}

void MappingProjector::renderPartialPano(Mat& outImg, vector<Mat> frames, Rect renderArea, Mat renderMask) {
	boost::timer::cpu_timer boostTimer;

	outImg = Mat::zeros(OUTPUT_PANO_HEIGHT, OUTPUT_PANO_WIDTH, CV_8UC3);
	int y1 = renderArea.tl().y;
	int y2 = renderArea.tl().y + renderArea.size().height;
	int x1 = renderArea.tl().x;
	int x2 = renderArea.tl().x + renderArea.size().width;

	if ( mEP->needFeed() ) 
		genExpoBlendingMap(frames);

	#pragma omp parallel for collapse(2) 
	for (int y=y1; y<y2; y++) {
		for (int x=x1; x<x2; x++) {
			for (int v=0; v<mViewCount; v++) {
				if (mProjMasks[v].at<uchar>(y, x) != 0) {
					int px = mProjMapX[v].at<int>(y, x);
					int py = mProjMapY[v].at<int>(y, x);
					if ( !(py < 0 || px < 0 || px >= mViewSize.width || py >= mViewSize.height) )
						outImg.at<Vec3b>(y, x) += frames[v].at<Vec3b>(py, px) * mFinalBlendingMap[v].at<float>(y, x);
				}
			}
		}
	}

	boostTimer.stop();
	mExecTimes.push_back( stod(boostTimer.format(3, "%w")) );
}

void MappingProjector::renderSaliencyArea(Mat& outImg, vector<Mat> frames, Mat saliencyInfo) {
	boost::timer::cpu_timer boostTimer;

	if ( mEP->needFeed() )
		genExpoBlendingMap(frames);

	int h = saliencyInfo.rows;
	int w = saliencyInfo.cols;

	#pragma omp parallel for collapse(2) 
	for (int y=0 ;y<h; y++) {
		for (int x=0; x<w; x++) {
			if (saliencyInfo.at<uchar>(y, x) == 0) 
				continue;
			for (int y0 = y*OUTPUT_PANO_HEIGHT/h, counterY=0; counterY <= OUTPUT_PANO_HEIGHT/h; y0++, counterY++) {
				for (int x0 = x*OUTPUT_PANO_WIDTH/w, counterX=0; counterX <= OUTPUT_PANO_WIDTH/w; x0++, counterX++) {
					outImg.at<Vec3b>(y0, x0) = Vec3b(0, 0, 0);
					for (int v=0; v<mViewCount; v++) {
						if (mProjMasks[v].at<uchar>(y0, x0) != 0) {
							int px = mProjMapX[v].at<int>(y0, x0);
							int py = mProjMapY[v].at<int>(y0, x0);
							if ( !(py < 0 || px < 0 || px >= mViewSize.width || py >= mViewSize.height) )
								outImg.at<Vec3b>(y0, x0) += frames[v].at<Vec3b>(py, px) * mFinalBlendingMap[v].at<float>(y0, x0);
						}
					}			
				}
			}
		}
	}
	boostTimer.stop();
	mExecTimes.push_back( stod(boostTimer.format(3, "%w")) );
}

void MappingProjector::renderSmallSizePano(Mat& outImg, vector<Mat> frames) {
	boost::timer::cpu_timer boostTimer;

	outImg = Mat::zeros(DOWN_SAMPLE_MAP_HEIGHT, DOWN_SAMPLE_MAP_WIDTH, CV_8UC3);
	float ratioX = (float) OUTPUT_PANO_WIDTH / DOWN_SAMPLE_MAP_WIDTH;
	float ratioY = (float) OUTPUT_PANO_HEIGHT / DOWN_SAMPLE_MAP_HEIGHT;

	if ( mEP->needFeed() )
		genExpoBlendingMap(frames);

	#pragma omp parallel for collapse(2) 
	for (int y=0; y<DOWN_SAMPLE_MAP_HEIGHT; y++) {
		for (int x=0; x<DOWN_SAMPLE_MAP_WIDTH; x++) {
			int oriY = (int) (y * ratioY);
			int oriX = (int) (x * ratioX);

			for (int v=0; v<mViewCount; v++) {
				if (mProjMasks[v].at<uchar>(oriY, oriX) != 0) {
					int px = mProjMapX[v].at<int>(oriY, oriX);
					int py = mProjMapY[v].at<int>(oriY, oriX);

					if ( !(py < 0 || px < 0 || px >= mViewSize.width || py >= mViewSize.height) ) {
						outImg.at<Vec3b>(y, x) += frames[v].at<Vec3b>(py, px) * mFinalBlendingMap[v].at<float>(oriY, oriX);
					}
				}
				
			}
		}
	}
}


vector<Vec3b> MappingProjector::getPixelsValueByUV(float u, float v, vector<Mat> frames, Mat& mask) {
	vector<Vec3b> outputPixels;

	int checkupX = static_cast<int>( (u + M_PI) * OUTPUT_PANO_WIDTH / (2*M_PI) );
	int checkupY = static_cast<int>( v * OUTPUT_PANO_HEIGHT / M_PI );

	for (int view=0; view<mViewCount; view++) {
		if ( mProjMasks[v].at<uchar>(checkupY, checkupX) == 0) {
			outputPixels.push_back( Vec3b(0, 0, 0) );
			mask.at<uchar>(0, view) = 0;
		}
		else {
			int px = mProjMapX[view].at<int>(checkupY, checkupX);
			int py = mProjMapY[view].at<int>(checkupY, checkupX);
			outputPixels.push_back( frames[view].at<Vec3b>(py, px) );
			mask.at<uchar>(0, view) = 1;
		}
	}
	return outputPixels;
}

int MappingProjector::rad2Deg(float r) {
	return static_cast<int> ( r * 180 / M_PI );
}

float MappingProjector::deg2Rad(int d) {
	return d * M_PI / 180 ;
}

void MappingProjector::saveMatchInfos(vector<MatchInfo> matchInfos) {
	mMatchInfos = matchInfos;
}

void MappingProjector::drawMatches(Mat& img) {
	vector<Scalar> colors;
	for(int v=0; v<mViewCount; v++)
		colors.push_back(Scalar(rand() % 255, rand() % 255, rand() % 255));

	for (size_t i=0; i<mMatchInfos.size(); i++) {
		int idx1 = mMatchInfos[i].idx1;
		int idx2 = mMatchInfos[i].idx2;

		vector<FeatureMatch> matches = mMatchInfos[i].matches;
		for (size_t m=0; m<matches.size(); m++) {
			Point2f p1 = mSphericalWarpers[idx1]->warpPoint( Point2f(matches[m].p1.x, matches[m].p1.y), mK[idx1], mR[idx1] );
			Point2f p2 = mSphericalWarpers[idx2]->warpPoint( Point2f(matches[m].p2.x, matches[m].p2.y), mK[idx2], mR[idx2] );

			int oX = static_cast<int> ( (p1.x - (-M_PI)) * OUTPUT_PANO_WIDTH / (2 * M_PI) ) % OUTPUT_PANO_WIDTH;
			int oY = static_cast<int> ( p1.y * OUTPUT_PANO_HEIGHT / (M_PI) )  % OUTPUT_PANO_HEIGHT;
			int oX2 = static_cast<int> ( (p2.x - (-M_PI)) * OUTPUT_PANO_WIDTH / (2 * M_PI) ) % OUTPUT_PANO_WIDTH;
			int oY2 = static_cast<int> ( p2.y * OUTPUT_PANO_HEIGHT / (M_PI) )  % OUTPUT_PANO_HEIGHT;
			
			circle(img, Point(oX, oY), 3, colors[idx1]);
			circle(img, Point(oX2, oY2), 3, colors[idx2]);
			line(img, Point(oX, oY), Point(oX2, oY2), Scalar(0, 0, 0), 2);

			circle(mWarpedImgs[idx1], Point(oX, oY), 3, colors[idx1]);
			circle(mWarpedImgs[idx2], Point(oX2, oY2), 3, colors[idx2]);
		}
	}	

	for (int v=0; v<mViewCount; v++) {
		imwrite(stringFormat("Warped_%d.png", v), mWarpedImgs[v]);
	}
}