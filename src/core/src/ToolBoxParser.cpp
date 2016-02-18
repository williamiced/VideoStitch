#include <header/ToolBoxParser.h>

// Intrinsic parameters
vector< float > ToolBoxParser::mX; // xi
vector< Point2f > ToolBoxParser::mF; // Focal length
vector< float > ToolBoxParser::mAr; // Aspect ratio
vector< Point > ToolBoxParser::mC; // Principle point
vector< Mat > ToolBoxParser::mD; // Distortion coefficients
vector< Mat > ToolBoxParser::mK; // Intrinsic Matrix

// Extrinsic parameters
vector< Mat > ToolBoxParser::mR; // Rotation matrix
vector< Mat > ToolBoxParser::mT; // Translation matrix


void ToolBoxParser::parseToolBoxFile(char* fileName) {
	ifstream tbFile(fileName);

	string lineStr;
	bool isIntrinsic = false;
	vector<float> tmpVec;
	while ( getline(tbFile, lineStr) ) {
		// If it is a camera index declaration
		if (lineStr.find("Camera #") == 0) 
			continue;

		// If it is a intrinsic / extrinsic declaration
		if (lineStr.find("Intrinsics") != string::npos) {
			isIntrinsic = true;
			continue;
		}
		else if (lineStr.find("Extrinsics") != string::npos) {
			isIntrinsic = false;
			continue;
		}

		// If it is in intrinsic session
		if (isIntrinsic) {
			vector<float> infos = parseIntrinsicInfo(lineStr);
			if (lineStr.find("xi") != string::npos)
				ToolBoxParser::mX.push_back( infos[0] );
			else if (lineStr.find("Focal length") != string::npos)
				ToolBoxParser::mF.push_back( Point2f(infos[0], infos[1]) );
			else if (lineStr.find("Aspect ratio") != string::npos)
				ToolBoxParser::mAr.push_back( infos[0] );
			else if (lineStr.find("Principle Point") != string::npos)
				ToolBoxParser::mC.push_back( Point2f(infos[0], infos[1]) );
			else if (lineStr.find("Distortion Coeff") != string::npos) 
				ToolBoxParser::mD.push_back( Mat( infos ) );
		} 
		// If it is in extrinsic session
		else {
			vector<float> infos = parseExtrinsicInfo(lineStr);
			for (unsigned int i=0; i<infos.size(); i++)
				tmpVec.push_back(infos[i]);
			if (tmpVec.size() >= 16) {
				Mat RT( tmpVec );
				RT = RT.reshape(1, 4).rowRange(0, 3).clone();
				ToolBoxParser::mR.push_back( RT.colRange(0, 3) );
				ToolBoxParser::mT.push_back( RT.colRange(3, 4) );
				tmpVec.clear();
			}
		}
	}

	for (unsigned int v=0; v<mF.size(); v++) {
		Mat K = Mat::zeros(3, 3, CV_32F);
		K.at<float>(0, 0) = static_cast<float> ( mF[v].x );
		K.at<float>(1, 1) = static_cast<float> ( mF[v].y );
		K.at<float>(0, 2) = static_cast<float> ( mC[v].x );
		K.at<float>(1, 2) = static_cast<float> ( mC[v].y );
		K.at<float>(2, 2) = 1.f;
		mK.push_back(K);
	}

}

vector<float> ToolBoxParser::parseIntrinsicInfo(string lineStr) {

	int colonPos = lineStr.find(":");
	string valueStr = lineStr.substr(colonPos+1);
	valueStr = removeSpaces( valueStr );
	valueStr = removeBrackets( valueStr );

	vector<float> returnVec;
	typedef boost::tokenizer< boost::char_separator<char> > tokenizer;
	boost::char_separator<char> sep{","};
	tokenizer tok{valueStr, sep};

	for (const auto &t : tok) 
		returnVec.push_back( stof(t) );

	return returnVec;
}

vector<float> ToolBoxParser::parseExtrinsicInfo(string valueStr) {	
	vector<float> returnVec;
	typedef boost::tokenizer< boost::char_separator<char> > tokenizer;
	boost::char_separator<char> sep{" "};
	tokenizer tok{valueStr, sep};

	for (const auto &t : tok) {
		if (t.length() > 0)
			returnVec.push_back( stof(t) );
	}

	return returnVec;
}

string ToolBoxParser::removeSpaces(string input) {
	input.erase( remove( input.begin(), input.end(), ' '), input.end() );
	return input;	
}

string ToolBoxParser::removeBrackets(string input) {
	input.erase( remove( input.begin(), input.end(), '['), input.end() );
	input.erase( remove( input.begin(), input.end(), ']'), input.end() );
	return input;
}