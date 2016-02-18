#include "UsageGUI.h"

QImage Mat2QImage(cv::Mat const& src) {
     cv::Mat temp; // make the same cv::Mat
     cvtColor(src, temp, CV_BGR2RGB); // cvtColor Makes a copt, that what i need
     QImage dest((const uchar *) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
     dest.bits(); // enforce deep copy, see documentation
     // of QImage::QImage ( const uchar * data, int width, int height, Format format )
     return dest;
}
