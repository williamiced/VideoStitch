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
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

    private slots:
        void updateImage();
        void on_pushButton_clicked();

    private:
        Ui::MainWindow *ui;
        unique_ptr<VideoStitcher> mVS;
        QTimer* mUpdateTimer;

        void doEnvSet();
        void launchStitcher();
        void switchUpdateTimer(bool turnOn);
};

#endif // MAINWINDOW_H
