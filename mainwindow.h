#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "QtMultimedia"
#include <QtMultimediaWidgets>
#include <QMainWindow>
#include <QImage>
#include <memory>
#include <string>
#include <jetsonGPIO.h>
#include "camera.h"
#include "trigger.h"
#include "ui_mainwindow.h"
#undef Bool

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent=0);

private slots:

    // Push Buttons
    void on_stopButton_clicked();
    void on_exitButton_clicked();
    void on_pauseButton_clicked(bool checked);

    // Display Data
    void displayQDefectImage(QImage, int);
    void displayQPrevDefectImage(QImage, int);
    void displayRes(int, int);
    void displayFrameRate(double, int);
    void displayCurrFrameRate(double, int);
    void displayExposureVal(int, int);
    void displayGainVal(int, int);

    void startCamerasCts();
//    void startCamerasTgr();

public slots:
    void camerasFinished();

private:

    void assignLabels();
    void connectStartSignalsSlots();
    void setupUiLayout();

    std::vector<bool> camerasRunning;

    Ui_MainWindow *ui;

    std::vector<Camera *> TX2Cameras;
    std::vector<QThread *> TX2CameraThreads;
    Trigger *trigger;

    QImage image;
    QImage defectImage;
    int numTX2Cameras = 3;
    double BGR[];
    int frameFinished = 0;
    QMutex *mutex;

    std::vector<QLabel *> images;
    std::vector<QLabel *> defectImages;
    std::vector<QLabel *> prevDefectImages;
    std::vector<QLabel *> resolutions;
    std::vector<QLabel *> frameRates;
    std::vector<QLabel *> currFrameRates;
    std::vector<QLabel *> exposureValues;
    std::vector<QLabel *> gainValues;

    std::vector<QSlider *> exposureSliders;
    std::vector<QSlider *> gainSliders;
};

#endif // MAINWINDOW_H

