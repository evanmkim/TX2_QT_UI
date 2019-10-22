#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <string>
#include <sstream>
#include <QString>
#include <algorithm>
#include <QKeyEvent>



MainWindow::MainWindow(QWidget *parent) :

    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    ///THREAD INITIALIZATION

    for (int i = 0; i < this->numTX2Cameras; i++) {
        this->TX2Cameras.push_back(std::unique_ptr<Camera>(new Camera(i)));
    }
    this->trigger = std::unique_ptr<Trigger>(new Trigger);

    for (int i = 0; i < this->numTX2Cameras; i++) {
        connect(ui->stopButton, &QPushButton::clicked, this->TX2Cameras[i].get(), &Camera::stopRequest);
        connect(ui->pauseButton, &QPushButton::clicked, this->TX2Cameras[i].get(), &Camera::pauseRequest);
        connect(ui->captureButton, &QPushButton::clicked, this->TX2Cameras[i].get(), &Camera::saveRequest);

        connect(this->TX2Cameras[i].get(),&Camera::returnQImage,this,&MainWindow::displayQImage);
        connect(this->TX2Cameras[i].get(),&Camera::returnQDefectImage,this,&MainWindow::displayQDefectImage);

        connect(this->TX2Cameras[i].get(),&Camera::returnRes,this,&MainWindow::displayRes);
        connect(this->TX2Cameras[i].get(),&Camera::returnFrameRate,this,&MainWindow::displayFrameRate);
        connect(this->TX2Cameras[i].get(),&Camera::returnCurrFrameRate,this,&MainWindow::displayCurrFrameRate);

        // UI Trigger
        //connect(ui->triggerButton, &PushButton::clicked, this->TX2Cameras[i].get(), &Camera::frameRequest);

        // Hardware Trigger
        connect(this->trigger.get(), &Trigger::captureRequest, this->TX2Cameras[i].get(), &Camera::frameRequest);
        connect(this->TX2Cameras[i].get(), &Camera::returnFrameFinished, this->trigger.get(), &Trigger::captureComplete);

        // UI Label Vector Assignment
        images.push_back(ui->QImageLabel1);
        images.push_back(ui->QImageLabel2);
        images.push_back(ui->QImageLabel3);

        defectImages.push_back(ui->QImageLabel1);
        defectImages.push_back(ui->QImageLabel2);
        defectImages.push_back(ui->QImageLabel3);

        resolutions.push_back(ui->labelResolution);
        resolutions.push_back(ui->labelResolution_2);
        resolutions.push_back(ui->labelResolution_3);

        frameRates.push_back(ui->labelFrameRate);
        frameRates.push_back(ui->labelFrameRate_2);
        frameRates.push_back(ui->labelFrameRate_3);

        currFrameRates.push_back(ui->labelCurrFrameRate);
        currFrameRates.push_back(ui->labelCurrFrameRate_2);
        currFrameRates.push_back(ui->labelCurrFrameRate_3);
    }
    connect(ui->stopButton, &QPushButton::clicked, this->trigger.get(), &Trigger::stopRequest);
    connect(ui->pauseButton, &QPushButton::clicked, this->trigger.get(), &Trigger::pauseRequest);

    ui->pauseButton->setCheckable(true);
}


MainWindow::~MainWindow()
{
    delete ui;
}

// Start into continuous capture at 30 fps (for live view)
void MainWindow::on_ctsModeStartButton_clicked()
{
    for (int i = 0; i < this->numTX2Cameras; i++) {
        this->TX2Cameras[i]->start();
        this->TX2Cameras[i]->captureMode = 0;
    }
}

// Start into triggered capture, hardware synchronized (for sync view)
void MainWindow::on_tgrModeStartButton_clicked()
{
    this->trigger->start();
    for (int i = 0; i < this->numTX2Cameras; i++) {
        this->TX2Cameras[i]->start();
        this->TX2Cameras[i]->captureMode = 1;
    }
}


void MainWindow::on_exitButton_clicked()
{
    this->close();
}

void MainWindow::on_pauseButton_clicked(bool checked)
{
    if (checked) {
        ui->pauseButton->setText("Resume");
    }
    else {
        cout << "resume button pressed" << endl;
        ui->pauseButton->setText("Pause");
    }
}

void MainWindow::displayQImage(QImage img_temp, int camIndex)
{
    image=img_temp;
    QMatrix rm;
    rm.rotate(0);
    this->images[camIndex]->setPixmap(QPixmap::fromImage(image).transformed(rm).scaled(this->images[camIndex]->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::displayQDefectImage(QImage img_temp, int camIndex)
{
    image=img_temp;
    QMatrix rm;
    rm.rotate(0);
    this->defectImages[camIndex]->setPixmap(QPixmap::fromImage(image).transformed(rm).scaled(this->defectImages[camIndex]->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::displayRes(int sensorRes, int camIndex)
{
    QString strExpTime=QString::number(sensorRes)+" p";
    this->resolutions[camIndex]->setText(strExpTime);
}

void MainWindow::displayFrameRate(double frameRate, int camIndex) {
    QString strFrameRate=QString::number(frameRate)+"  fps";
    this->frameRates[camIndex]->setText(strFrameRate);
}

void MainWindow::displayCurrFrameRate(double currFrameRate, int camIndex) {
    QString strFrameRate=QString::number(currFrameRate)+"  fps";
    this->currFrameRates[camIndex]->setText(strFrameRate);
}

