#include "mainwindow.h"
#include "ui_mainwindow.h"

Ui::MainWindow* MainWindow::ui = new Ui::MainWindow;

int MainWindow::mAnchorX = -1;
int MainWindow::mAnchorY = -1;
float MainWindow::mRenderTLx = 0;
float MainWindow::mRenderTLy = M_PI/2;
float MainWindow::mFOVx = M_PI/2;
float MainWindow::mFOVy = M_PI/2;

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
    string inputStr = "bin/VideoStitch --input data/Cut15/inputVideo.txt --calibration data/Cut15/Calibration.txt --pto data/Cut15/15.pto --duration 50 --output StitchResult.avi";
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

void MainWindow::updateRenderRegion(float& u1, float& u2, float& v1, float& v2) {
    u1 = mRenderTLx;
    u2 = (mRenderTLx + mFOVx) < M_PI ? (mRenderTLx + mFOVx) : (mRenderTLx + mFOVx - 2*M_PI);
    v1 = mRenderTLy;
    v2 = (mRenderTLy + mFOVy) < M_PI ? (mRenderTLy + mFOVy) : (mRenderTLy + mFOVy - M_PI);
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    mAnchorX = event->x();
    mAnchorY = event->y();
    mIsMousePressed = true;
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    mIsMousePressed = false;
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    if (!mIsMousePressed)
        return;
    float offsetX = (event->x() - mAnchorX) / 13000.f;
    float offsetY = (event->y() - mAnchorY) / 13000.f;

    mRenderTLx = mRenderTLx - offsetX;
    while(mRenderTLx >= M_PI) mRenderTLx -= 2*M_PI;
    while(mRenderTLx < -M_PI) mRenderTLx += 2*M_PI;

    mRenderTLy = mRenderTLy - offsetY;
    while(mRenderTLy >= M_PI) mRenderTLy -= M_PI;
    while(mRenderTLy < 0) mRenderTLy += M_PI;
}
