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

    void on_ctsModeStartButton_clicked();
    void on_tgrModeStartButton_clicked();
    void on_exitButton_clicked();
    void on_pauseButton_clicked(bool checked);

    void displayQImage(QImage, int);
    void displayQDefectImage(QImage, int);

    void displayRes(int, int);

private:
    Ui::MainWindow *ui;
    std::vector<std::unique_ptr<Camera>> TX2Cameras;
    std::unique_ptr<Trigger> trigger;
    QImage image;
    QImage defectImage;
    int numTX2Cameras = 3;
    double BGR[];

    QLabel *images[3];
    QLabel *defectImages[3];
    QLabel *resolutions[3];
};

#endif // MAINWINDOW_H

