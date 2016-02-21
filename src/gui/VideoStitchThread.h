#ifndef VIDEOSTITCHTHREAD_H
#define VIDEOSTITCHTHREAD_H

#include <QThread>
#include <QImage>
#include <QPixmap>
#include <header/VideoStitch.h>
#include "UsageGUI.h"

class VideoStitchThread: public QThread {
    Q_OBJECT

    public:
        VideoStitchThread(renderRegionUpdaterPtr updateRenderRegion, int c, char** v);
        static void updateImage(cv::Mat);
        static VideoStitchThread* mVSthr;

    protected:
        void run();

    private:
        int argc;
        char** argv;
        static unique_ptr<VideoStitcher> mVS;
        static void emitUpdateSignal(QPixmap qp);

    signals:
        void requestUpdateImage(const QPixmap qp);
};

#endif // VIDEOSTITCHTHREAD_H
