#include <header/BlendingProcessor.h>

void BlendingProcessor::doBlending( vector<Mat> warpedImg, Mat& result, Mat& resultMask ) {
    prepare( cv::detail::resultRoi(mCorners, mSizes) );

	for (int v=0; v<mViewCount; v++) {
    	Mat warped_s;
    	warpedImg[v].convertTo(warped_s, CV_16S);
    	feed(warped_s, mDilateMasks[v], mCorners[v]);
	}

    blend(result, resultMask);
    result.convertTo(result, CV_8U);
}

void BlendingProcessor::updateMasks(vector<Mat> masks) {
    mDilateMasks.resize(mViewCount);
    // Save dilate masks
    for (int v=0; v<mViewCount; v++) {
        Mat mask_warped = masks[v].clone();
        Mat dilated_mask, seam_mask;
        dilate(mask_warped, dilated_mask, Mat());
        cv::resize(dilated_mask, seam_mask, mask_warped.size());
        mask_warped = seam_mask & mask_warped;
        mDilateMasks[v] = mask_warped;
    }
}

BlendingProcessor::BlendingProcessor( int vc, Rect canvasROI, vector<Point> c, vector<Size> s) : mViewCount(vc), mCanvasROI(canvasROI), mCorners(c), mSizes(s) {
	// Calculate blending strength
	float blendStrength = 5.f;
	float blendWidth = sqrt(static_cast<float>(mCanvasROI.area())) * blendStrength / 100.f;
    setSharpness( 1.f/blendWidth );
}

BlendingProcessor::~BlendingProcessor() {
	
}

void BlendingProcessor::prepare(Rect dst_roi) {
    Blender::prepare(dst_roi);
    dst_weight_map_.create(dst_roi.size(), CV_32F);
    dst_weight_map_.setTo(0);
}

void BlendingProcessor::feed (InputArray _img, InputArray mask, Point tl) {
    Mat img = _img.getMat();
    Mat dst = dst_.getMat(ACCESS_RW);

    CV_Assert(img.type() == CV_16SC3);
    CV_Assert(mask.type() == CV_8U);

    cv::detail::createWeightMap(mask, sharpness_, weight_map_);
    Mat weight_map = weight_map_.getMat(ACCESS_READ);
    Mat dst_weight_map = dst_weight_map_.getMat(ACCESS_RW);

    int dx = tl.x - dst_roi_.x;
    int dy = tl.y - dst_roi_.y;

    for (int y = 0; y < img.rows; ++y) {
        const Point3_<short>* src_row = img.ptr<Point3_<short> >(y);
        Point3_<short>* dst_row = dst.ptr<Point3_<short> >(dy + y);
        const float* weight_row = weight_map.ptr<float>(y);
        float* dst_weight_row = dst_weight_map.ptr<float>(dy + y);

        for (int x = 0; x < img.cols; ++x)
        {
            dst_row[dx + x].x += static_cast<short>(src_row[x].x * weight_row[x]);
            dst_row[dx + x].y += static_cast<short>(src_row[x].y * weight_row[x]);
            dst_row[dx + x].z += static_cast<short>(src_row[x].z * weight_row[x]);
            dst_weight_row[dx + x] += weight_row[x];
        }
    }
}

void BlendingProcessor::blend(InputOutputArray dst, InputOutputArray dst_mask) {
    static const float WEIGHT_EPS = 1e-5f;

    cv::detail::normalizeUsingWeightMap(dst_weight_map_, dst_);
    compare(dst_weight_map_, WEIGHT_EPS, dst_mask_, CMP_GT);
    Blender::blend(dst, dst_mask);
}
