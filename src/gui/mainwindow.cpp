#include "mainwindow.h"
#include "ui_mainwindow.h"

Ui::MainWindow* MainWindow::ui = new Ui::MainWindow;
unique_ptr<VideoStitcher> MainWindow::mVS = nullptr;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent) {

    ui->setupUi(this);
    doEnvSet();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_pushButton_clicked() {
    launchStitcher();

    mVS->registerCallbackFunc ( updateImage );
    mVS->doRealTimeStitching(argc, argv);

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

    mVS = unique_ptr<VideoStitcher> ( new VideoStitcher(argc, argv) );
}

void MainWindow::updateImage(cv::Mat img) {
    cout << "Update image" << endl;
    if (mVS == nullptr)
        return;
    QImage qimg = Mat2QImage(img);
    ui->mainImage->setPixmap(QPixmap::fromImage(qimg));
    ui->mainImage->repaint();
}
