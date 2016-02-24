#include "mainwindow.h"
#include "ui_mainwindow.h"

Ui::MainWindow* MainWindow::ui = new Ui::MainWindow;

int MainWindow::mAnchorX = -1;
int MainWindow::mAnchorY = -1;
float MainWindow::mAnchorCenterX = 0.f;
float MainWindow::mAnchorCenterY = M_PI/2.f;
float MainWindow::mCurrentCenterX = 0;
float MainWindow::mCurrentCenterY = M_PI/2.f;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mIsMousePressed(false) {

    ui->setupUi(this);
    doEnvSet();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_pushButton_clicked() {
    launchStitcher();
    mVSthr = unique_ptr<VideoStitchThread>( new VideoStitchThread( updateRenderRegion, argc, argv) );
    connect(&(*mVSthr), SIGNAL(requestUpdateImage(QPixmap)), this, SLOT(updateImageOnUI(QPixmap)));
    mVSthr->start();
    //mVS.release();
    //mVS = nullptr;
    //mUpdateTimer = nullptr;
}

void MainWindow::doEnvSet() {
    chdir("..");
    std::cout.setf(std::ios::unitbuf);
}

void MainWindow::launchStitcher() {
    string inputStr = "bin/VideoStitch --input data/Cut15/inputVideo.txt --calibration data/Cut15/Calibration.txt --pto data/Cut15/15.pto --duration 100 --output StitchResult.avi";
    vector<string> inputs;
    boost::split(inputs, inputStr, boost::is_any_of(" "));

    argc = inputs.size();
    argv = new char*[inputs.size()];

    for(size_t i = 0; i < inputs.size(); i++) {
        argv[i] = new char[inputs[i].length() + 1];
        strcpy(argv[i], inputs[i].c_str());
    }
}

void MainWindow::updateImageOnUI(QPixmap qp) {
    ui->mainImage->setPixmap(qp);
    ui->mainImage->repaint();
}

void MainWindow::updateRenderRegion(float& centerU, float& centerV, float& range) {
    centerU = mCurrentCenterX;
    centerV = mCurrentCenterY;
    range = 1.f;
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    mAnchorX = event->x();
    mAnchorY = event->y();
    mAnchorCenterX = mCurrentCenterX;
    mAnchorCenterY = mCurrentCenterY;
    mIsMousePressed = true;
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    mIsMousePressed = false;
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    if (!mIsMousePressed)
        return;
    float offsetX = (event->x() - mAnchorX) / 1000.f;
    float offsetY = (event->y() - mAnchorY) / 1000.f;

    mCurrentCenterX = mAnchorCenterX - offsetX;
    mCurrentCenterY = mAnchorCenterY - offsetY;

    while (mCurrentCenterY < 0) {
        mCurrentCenterY = fabs(mCurrentCenterY);
        mCurrentCenterX += M_PI;
    }
    while (mCurrentCenterY > M_PI) {
        mCurrentCenterY = fabs(2*M_PI - mCurrentCenterY);
        mCurrentCenterX += M_PI;
    }
    while (mCurrentCenterX < -M_PI)
        mCurrentCenterX += 2*M_PI;
    while (mCurrentCenterX > M_PI)
        mCurrentCenterX -= 2*M_PI;

    /*
    if (mRenderTLy > M_PI) {
        mRenderTLy = fabs(M_PI - mRenderTLy);
        mRenderTLx = mRenderTLx + M_PI;
    }
    if (mRenderTLy < 0) {
        mRenderTLy = fabs(mRenderTLy);
        mRenderTLx = mRenderTLx + M_PI;
    }

    while (mRenderTLx > M_PI) mRenderTLx = mRenderTLx - 2*M_PI;
    while (mRenderTLx < -M_PI) mRenderTLx = mRenderTLx + 2*M_PI;
    */
}
