#include <header/BlendingProcessor.h>

void BlendingProcessor::doBlending( vector<Mat> warpedImg, Mat& result, Mat& resultMask ) {
    mBlender->prepare(mCorners, mSizes);

	for (int v=0; v<mViewCount; v++) {
    	Mat warped_s;
    	warpedImg[v].convertTo(warped_s, CV_16S);
    	mBlender->feed(warped_s, mDilateMasks[v], mCorners[v]);
	}

    mBlender->blend(result, resultMask);
    result.convertTo(result, CV_8U);
}

BlendingProcessor::BlendingProcessor( int vc, Rect canvasROI, vector<Point> c, vector<Size> s, vector<Mat> masks) : mViewCount(vc), mCanvasROI(canvasROI), mCorners(c), mSizes(s) {
	mBlender = cv::detail::Blender::createDefault(cv::detail::Blender::FEATHER, true);

	// Calculate blending strength
	float blendStrength = 5.f;
	float blendWidth = sqrt(static_cast<float>(mCanvasROI.area())) * blendStrength / 100.f;
	//cv::detail::MultiBandBlender* mb = dynamic_cast<cv::detail::MultiBandBlender*>( mBlender.get() );
    //mb->setNumBands(static_cast<int>(ceil(log( blendWidth ) / log(2.)) - 1.));
    cv::detail::FeatherBlender* fb = dynamic_cast<cv::detail::FeatherBlender*>( mBlender.get() );
    fb->setSharpness( 1.f/blendWidth );

    // Save dilate masks
    for (int v=0; v<mViewCount; v++) {
    	Mat mask_warped = masks[v].clone();
    	Mat dilated_mask, seam_mask;
    	dilate(mask_warped, dilated_mask, Mat());
        cv::resize(dilated_mask, seam_mask, mask_warped.size());
        mask_warped = seam_mask & mask_warped;
        mDilateMasks.push_back(mask_warped);
    }
}

BlendingProcessor::~BlendingProcessor() {
	
}