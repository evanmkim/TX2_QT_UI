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

    void on_startButton_clicked();
    void on_exitButton_clicked();

    void displayQImage1(QImage);
    void displayDefectImage1(QImage);
    void displayQImage2(QImage);
    void displayDefectImage2(QImage);
    void displayQImage3(QImage);
    void displayDefectImage3(QImage);

private:
    Ui::MainWindow *ui;
    std::vector<std::unique_ptr<Camera>> TX2Cameras;
    std::unique_ptr<Trigger> trigger;
    QImage image;
    QImage defectImage;
    int numTX2Cameras = 3;
    QLabel *images[3];
    QLabel *defectImages[3];
    double BGR[];
};

#endif // MAINWINDOW_H

