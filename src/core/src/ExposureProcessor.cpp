#include <header/ExposureProcessor.h>

void ExposureProcessor::doExposureCompensate( vector<Mat> warpedImg, vector<Mat> warpedMasks) {
    SETUP_TIMER

	#pragma omp parallel for collapse(1)
	for (int v=0; v<mViewCount; v++) 
		apply(v, mCorners[v], warpedImg[v], warpedMasks[v]);
}

void ExposureProcessor::feedExposures(vector<Mat> warpedImg, vector<Mat> warpedMasks) {
	vector<UMat> imgs, masks;
	for (int v=0; v<mViewCount; v++) {
		imgs.push_back(warpedImg[v].getUMat(ACCESS_RW));
		masks.push_back(warpedMasks[v].getUMat(ACCESS_RW));
	}
	feed(mCorners, imgs, masks);
	mNeedFeed = false;
}

bool ExposureProcessor::needFeed() {
	return mNeedFeed;
}

ExposureProcessor::ExposureProcessor( vector<Point> c, int vc) : mCorners(c), mViewCount(vc) {
	mNeedFeed = true;

	logMsg(LOG_INFO, " [ Initialize Cuda ... ]");
	GpuMat m1(Mat(1, 1, CV_8UC1));
	GpuMat m2(Mat(1, 1, CV_8UC1));
	cv::cuda::add(m1, m2, m1);
	logMsg(LOG_INFO, " [ Done Cuda initialization ] ");
}

ExposureProcessor::~ExposureProcessor() {
	
}

void ExposureProcessor::feed(const std::vector<Point> &corners, const std::vector<UMat> &images,
                               const std::vector<UMat> &masks) {
	std::vector<std::pair<UMat,uchar> > level_masks;
    for (size_t i = 0; i < masks.size(); ++i)
        level_masks.push_back(std::make_pair(masks[i], 255));
    feed(corners, images, level_masks);
}

void ExposureProcessor::feed(const std::vector<Point> &corners, const std::vector<UMat> &images,
                           const std::vector<std::pair<UMat,uchar> > &masks) {
    CV_Assert(corners.size() == images.size() && images.size() == masks.size());

    const int num_images = static_cast<int>(images.size());
    Mat_<int> N(num_images, num_images); N.setTo(0);
    Mat_<double> I(num_images, num_images); I.setTo(0);

    //Rect dst_roi = resultRoi(corners, images);
    Mat subimg1, subimg2;
    Mat_<uchar> submask1, submask2, intersect;

    for (int i = 0; i < num_images; ++i) {
        for (int j = i; j < num_images; ++j) {
            Rect roi;
            if (cv::detail::overlapRoi(corners[i], corners[j], images[i].size(), images[j].size(), roi)) {
                subimg1 = images[i](Rect(roi.tl() - corners[i], roi.br() - corners[i])).getMat(ACCESS_READ);
                subimg2 = images[j](Rect(roi.tl() - corners[j], roi.br() - corners[j])).getMat(ACCESS_READ);

                submask1 = masks[i].first(Rect(roi.tl() - corners[i], roi.br() - corners[i])).getMat(ACCESS_READ);
                submask2 = masks[j].first(Rect(roi.tl() - corners[j], roi.br() - corners[j])).getMat(ACCESS_READ);
                intersect = (submask1 == masks[i].second) & (submask2 == masks[j].second);

                N(i, j) = N(j, i) = std::max(1, cv::countNonZero(intersect));

                double Isum1 = 0, Isum2 = 0;
                for (int y = 0; y < roi.height; ++y) {
                    const Point3_<uchar>* r1 = subimg1.ptr<Point3_<uchar> >(y);
                    const Point3_<uchar>* r2 = subimg2.ptr<Point3_<uchar> >(y);
                    for (int x = 0; x < roi.width; ++x) {
                        if (intersect(y, x)) {
                            Isum1 += std::sqrt(static_cast<double>(cv::detail::sqr(r1[x].x) + cv::detail::sqr(r1[x].y) + cv::detail::sqr(r1[x].z)));
                            Isum2 += std::sqrt(static_cast<double>(cv::detail::sqr(r2[x].x) + cv::detail::sqr(r2[x].y) + cv::detail::sqr(r2[x].z)));
                        }
                    }
                }
                I(i, j) = Isum1 / N(i, j);
                I(j, i) = Isum2 / N(i, j);
            }
        }
    }

    double alpha = 0.01;
    double beta = 100;

    Mat_<double> A(num_images, num_images); A.setTo(0);
    Mat_<double> b(num_images, 1); b.setTo(0);
    for (int i = 0; i < num_images; ++i)
    {
        for (int j = 0; j < num_images; ++j)
        {
            b(i, 0) += beta * N(i, j);
            A(i, i) += beta * N(i, j);
            if (j == i) continue;
            A(i, i) += 2 * alpha * I(i, j) * I(i, j) * N(i, j);
            A(i, j) -= 2 * alpha * I(i, j) * I(j, i) * N(i, j);
        }
    }
    solve(A, b, gains_);
}

void ExposureProcessor::apply(int index, Point /*corner*/, InputOutputArray image, InputArray /*mask*/) {
    Mat src = image.getMat().reshape(1);
    GpuMat srcImg( src );
	double target = gains_(index, 0);
	cv::cuda::multiply(srcImg, target, srcImg);
	srcImg.download(src);
    //double target = gains_(index, 0);
    //cv::multiply(image, target, image);
}

std::vector<double> ExposureProcessor::gains() const {
    std::vector<double> gains_vec(gains_.rows);
    for (int i = 0; i < gains_.rows; ++i)
        gains_vec[i] = gains_(i, 0);
    return gains_vec;
}
