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

    void startCamerasCts();
//    void startCamerasTgr();
    void setupFrameSettings(int);

    // Display Data
    void displayQDefectImage(QImage, int);
    void displayQPrevDefectImage(QImage, int);
    void displayRes(int, int);
    void displayFrameRate(double, int);
    void displayCurrFrameRate(double, int);

    // Push Buttons
    void stopAllRequest();
    void pauseAllRequest(bool);
    void captureAllRequest();
    void closeUi();

    // Radio Buttons
    void displayNormal(bool);
    void displayGreyscale(bool);
    void displayThreshold(bool);
    void displayFloodfill(bool);

    // Drop Downs
    void setGain(int);
    void setExposure(int);

signals:

    void restartCapture();

private:

    Ui_MainWindow *ui;

    const int numTX2Cameras = 3;

    int camerasFinished;
    std::vector<Camera *> TX2Cameras;
    std::vector<QThread *> TX2CameraThreads;
    Trigger *trigger;

    void assignLabels();
    void connectStart();
    void setupUiLayout();

    bool camerasRunning;

    QImage image;
    QImage defectImage;
    double BGR[];

    int gain;
    int exposure;

//    int frameFinished = 0;
//    QMutex *mutex;

    std::vector<QLabel *> images;
    std::vector<QLabel *> defectImages;
    std::vector<QLabel *> prevDefectImages;
    std::vector<QLabel *> resolutions;
    std::vector<QLabel *> frameRates;
    std::vector<QLabel *> currFrameRates;

    const int dropDownOptionCount = 10;
    std::vector<int> exposureValues;
    std::vector<int> gainValues;
};

#endif // MAINWINDOW_H

