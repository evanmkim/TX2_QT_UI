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
    ~MainWindow();

private slots:

    // Display Data
    void displayQDefectImage(QImage, int);
    void displayQPrevDefectImage(QImage, int);
    void displayRes(int, int);
    void displayFrameRate(double, int);
    void displayCurrFrameRate(double, int);

    void startCamerasCts();
//    void startCamerasTgr();

    void camerasFinished();

    void setGain(int);
    void setExposure(int);

    // Buttons
    void stopAllRequest();
    void pauseAllRequest(bool);
    void captureAllRequest();
    void exitRequest();


public slots:

    void setupFrameSettings(int);

private:

    Ui_MainWindow *ui;

    const int numTX2Cameras = 3;
    std::vector<Camera *> TX2Cameras;
    std::vector<QThread *> TX2CameraThreads;
    Trigger *trigger;

    void assignLabels();
    void connectStart();
    void setupUiLayout();

    int camerasRunning;

    int exposure;
    int gain;
    QImage image;
    QImage defectImage;
    double BGR[];

//    int frameFinished = 0;
//    QMutex *mutex;

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

