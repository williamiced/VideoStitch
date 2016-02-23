#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <cstdlib>
#include <iostream>
#include <QMainWindow>
#include <QLabel>
#include <QPixmap>
#include <QMouseEvent>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <string>
#include <unistd.h>
#include "UsageGUI.h"
#include "VideoStitchThread.h"

using namespace std;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
        static void updateImage(Mat img);
        static void updateRenderRegion(float& centerU, float& centerV, float& range);
        static int mAnchorX;
        static int mAnchorY;
        static float mAnchorCenterX;
        static float mAnchorCenterY;
        static float mCurrentCenterX;
        static float mCurrentCenterY;
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

    protected:
        void mouseMoveEvent(QMouseEvent *event);
        void mousePressEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);

    public slots:
        void updateImageOnUI(QPixmap qp);

    private slots:
        void on_pushButton_clicked();

    private:
        int argc;
        char** argv;
        static Ui::MainWindow *ui;
        unique_ptr<VideoStitchThread> mVSthr;

        // For Mouse events
        bool mIsMousePressed;

        void doEnvSet();
        void launchStitcher();
};

#endif // MAINWINDOW_H
