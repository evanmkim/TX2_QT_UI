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
#undef Bool
//SIGNAL

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    // Push Buttons
    void on_ctsModeStartButton_clicked();
    void on_tgrModeStartButton_clicked();
    void on_exitButton_clicked();
    void on_pauseButton_clicked(bool checked);

    // Display Data
    //void displayQImage(QImage, int);
    void displayQDefectImage(QImage, int);
    void displayQPrevDefectImage(QImage, int);
    void displayRes(int, int);
    void displayFrameRate(double, int);
    void displayCurrFrameRate(double, int);
    void displayExposureVal(int, int);
    void displayGainVal(int, int);

private:
    Ui::MainWindow *ui;
    std::vector<std::unique_ptr<Camera>> TX2Cameras;
    std::unique_ptr<Trigger> trigger;
    QImage image;
    QImage defectImage;
    int numTX2Cameras = 3;
    double BGR[];

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

