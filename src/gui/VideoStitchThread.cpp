#include "VideoStitchThread.h"

unique_ptr<VideoStitcher> VideoStitchThread::mVS = nullptr;
VideoStitchThread* VideoStitchThread::mVSthr = nullptr;

VideoStitchThread::VideoStitchThread(renderRegionUpdaterPtr updateRenderRegion, int c, char** v):
    argc(c),
    argv(v) {
    mVSthr = this;
    mVS = unique_ptr<VideoStitcher>( new VideoStitcher(argc, argv) );
    mVS->registerCallbackFunc ( updateImage );
    mVS->registerUpdaterFunc ( updateRenderRegion );
}

void VideoStitchThread::run() {
    mVS->doRealTimeStitching(argc, argv);
}

void VideoStitchThread::updateImage(cv::Mat img) {
    cout << "Update image" << endl;
    if (mVS == nullptr)
        return;
    QImage qimg = Mat2QImage(img);

    mVSthr->requestUpdateImage( QPixmap::fromImage(qimg) );
}

void VideoStitchThread::emitUpdateSignal(QPixmap qp) {
    //emit requestUpdateImage( qp );
}
