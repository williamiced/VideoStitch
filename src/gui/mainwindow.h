#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <cstdlib>
#include <iostream>
#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QImage>
#include <QPixmap>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <string>
#include <unistd.h>
#include <header/VideoStitch.h>
#include "UsageGUI.h"

using namespace std;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
        static void updateImage(Mat img);
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

    private slots:
        void on_pushButton_clicked();

    private:
        int argc;
        char** argv;
        static Ui::MainWindow *ui;
        static unique_ptr<VideoStitcher> mVS;

        void doEnvSet();
        void launchStitcher();
};

#endif // MAINWINDOW_H
