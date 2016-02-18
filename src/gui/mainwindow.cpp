#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow){
    ui->setupUi(this);

    mVS = nullptr;
    mUpdateTimer = nullptr;
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_pushButton_clicked() {
    doEnvSet();

    launchStitcher();

    switchUpdateTimer(true);
}

void MainWindow::doEnvSet() {
    chdir("..");
    std::cout.setf(std::ios::unitbuf);
}

void MainWindow::launchStitcher() {
    string inputStr = "bin/VideoStitch --input data/Cut15/inputVideo.txt --calibration data/Cut15/Calibration.txt --pto data/Cut15/15.pto --duration 50 --output StitchResult.avi";
    vector<string> inputs;
    boost::split(inputs, inputStr, boost::is_any_of(" "));

    int argc = inputs.size();
    char** argv = new char*[inputs.size()];

    for(size_t i = 0; i < inputs.size(); i++) {
        argv[i] = new char[inputs[i].length() + 1];
        strcpy(argv[i], inputs[i].c_str());
    }

    mVS = unique_ptr<VideoStitcher> ( new VideoStitcher(argc, argv) );
    mVS->doRealTimeStitching(argc, argv);
    mVS.release();
}

void MainWindow::switchUpdateTimer(bool turnOn) {
    if (mUpdateTimer == nullptr) {
        mUpdateTimer = new QTimer(this);
        connect(mUpdateTimer, SIGNAL(timeout()), this, SLOT(updateImage()));
    }
    if (turnOn)
        mUpdateTimer->start(50);
    else
        mUpdateTimer->stop();
}

void MainWindow::updateImage() {
    if (mVS == nullptr)
        return;
    cv::Mat img;
    if (mVS->askForImage(img) ) {
        QImage qimg = Mat2QImage(img);
        ui->mainImage->setPixmap(QPixmap::fromImage(qimg));
        ui->mainImage->show();
    }
}
