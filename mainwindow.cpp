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

    // UI Label Vector Assignment

    this->defectImages.push_back(ui->QDefectLabel);
    this->defectImages.push_back(ui->QDefectLabel_2);
    this->defectImages.push_back(ui->QDefectLabel_3);

    this->prevDefectImages.push_back(ui->QPrevDefectLabel);
    this->prevDefectImages.push_back(ui->QPrevDefectLabel_2);
    this->prevDefectImages.push_back(ui->QPrevDefectLabel_3);

    this->resolutions.push_back(ui->labelResolution);
    this->resolutions.push_back(ui->labelResolution_2);
    this->resolutions.push_back(ui->labelResolution_3);

    this->frameRates.push_back(ui->labelFrameRate);
    this->frameRates.push_back(ui->labelFrameRate_2);
    this->frameRates.push_back(ui->labelFrameRate_3);

    this->currFrameRates.push_back(ui->labelCurrFrameRate);
    this->currFrameRates.push_back(ui->labelCurrFrameRate_2);
    this->currFrameRates.push_back(ui->labelCurrFrameRate_3);

    this->exposureSliders.push_back(ui->ExposureTimeSlider);
    this->exposureSliders.push_back(ui->ExposureTimeSlider_2);
    this->exposureSliders.push_back(ui->ExposureTimeSlider_3);

    this->gainSliders.push_back(ui->GainSlider);
    this->gainSliders.push_back(ui->GainSlider_2);
    this->gainSliders.push_back(ui->GainSlider_3);

    this->exposureValues.push_back(ui->ExposureLabel);
    this->exposureValues.push_back(ui->ExposureLabel_2);
    this->exposureValues.push_back(ui->ExposureLabel_3);

    this->gainValues.push_back(ui->GainLabel);
    this->gainValues.push_back(ui->GainLabel_2);
    this->gainValues.push_back(ui->GainLabel_3);

    this->frameFinished = 0;

    // Thread Initialization
    for (int i = 0; i < this->numTX2Cameras; i++) {
        this->TX2Cameras.push_back(std::unique_ptr<Camera>(new Camera(i, &(this->mutex), &(this->frameFinished))));
    }
    this->trigger = std::unique_ptr<Trigger>(new Trigger);

    for (int i = 0; i < this->numTX2Cameras; i++) {

        // Buttons
        connect(ui->stopButton, &QPushButton::clicked, this->TX2Cameras[i].get(), &Camera::stopRequest);
        connect(ui->pauseButton, &QPushButton::clicked, this->TX2Cameras[i].get(), &Camera::pauseRequest);
        connect(ui->captureButton, &QPushButton::clicked, this->TX2Cameras[i].get(), &Camera::saveRequest);

        //Sliders
        connect(this->exposureSliders[i], &QSlider::valueChanged, this->TX2Cameras[i].get(), &Camera::setExposure);
        connect(this->gainSliders[i], &QSlider::valueChanged, this->TX2Cameras[i].get(), &Camera::setGain);

        // Display Data
        connect(this->TX2Cameras[i].get(),&Camera::returnQDefectImage,this,&MainWindow::displayQDefectImage);
        connect(this->TX2Cameras[i].get(),&Camera::returnQPrevDefectImage,this,&MainWindow::displayQPrevDefectImage);
        connect(this->TX2Cameras[i].get(),&Camera::returnRes,this,&MainWindow::displayRes);
        connect(this->TX2Cameras[i].get(),&Camera::returnFrameRate,this,&MainWindow::displayFrameRate);
        connect(this->TX2Cameras[i].get(),&Camera::returnCurrFrameRate,this,&MainWindow::displayCurrFrameRate);
        connect(this->TX2Cameras[i].get(),&Camera::returnExposureVal,this,&MainWindow::displayExposureVal);
        connect(this->TX2Cameras[i].get(),&Camera::returnGainVal,this,&MainWindow::displayGainVal);

        // UI Trigger
        //connect(ui->triggerButton, &PushButton::clicked, this->TX2Cameras[i].get(), &Camera::frameRequest);

        // Hardware Trigger
        connect(this->trigger.get(), &Trigger::captureRequest, this->TX2Cameras[i].get(), &Camera::frameRequest);
        connect(this->TX2Cameras[i].get(), &Camera::returnFrameFinished, this->trigger.get(), &Trigger::captureComplete);

        // Slider Elements
        this->exposureSliders[i]->setRange(30,1000);
        this->exposureSliders[i]->setValue(50);
        this->gainSliders[i]->setRange(1,250);
        this->gainSliders[i]->setValue(50);
    }
    connect(ui->stopButton, &QPushButton::clicked, this->trigger.get(), &Trigger::stopRequest);
    connect(ui->pauseButton, &QPushButton::clicked, this->trigger.get(), &Trigger::pauseRequest);

    ui->pauseButton->setCheckable(true);

    // Taymer Logo
    QString filename = "/home/nvidia/ExposureUIQt/Assets/0.png";
    ui->TaymerLogo->setAlignment(Qt::AlignCenter);
    QPixmap logo;

    if (logo.load(filename)) {
        logo = logo.scaled(ui->TaymerLogo->size(),Qt::KeepAspectRatio);
        ui->TaymerLogo->setPixmap(logo);
    } else {
        cout << "Unable to load, bad filename" << endl;
    }
}


MainWindow::~MainWindow()
{
    delete ui;
}

// Start into continuous capture at 30 fps (for live view)
void MainWindow::on_ctsModeStartButton_clicked()
{
    for (int i = 0; i < this->numTX2Cameras; i++) {
        this->TX2Cameras[i]->captureMode = 0;
        this->TX2Cameras[i]->start();
    }
}

// Start into triggered capture, hardware synchronized (for sync view)
void MainWindow::on_tgrModeStartButton_clicked()
{
    for (int i = 0; i < this->numTX2Cameras; i++) {
        this->TX2Cameras[i]->captureMode = 1;
        this->TX2Cameras[i]->start();
    }
    this->trigger->start();
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

void MainWindow::displayQDefectImage(QImage img_temp, int camIndex)
{
    image=img_temp;
    QMatrix rm;
    rm.rotate(0);
    this->defectImages[camIndex]->setPixmap(QPixmap::fromImage(image).transformed(rm).scaled(this->defectImages[camIndex]->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::displayQPrevDefectImage(QImage img_temp, int camIndex)
{
    image=img_temp;
    QMatrix rm;
    rm.rotate(0);
    this->prevDefectImages[camIndex]->setPixmap(QPixmap::fromImage(image).transformed(rm).scaled(this->defectImages[camIndex]->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
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

void MainWindow::displayExposureVal(int newValue, int camIndex)
{
    QString strExpTime=QString::number(newValue);
    this->exposureValues[camIndex]->setText(strExpTime+" µs");
}

void MainWindow::displayGainVal(int newValue, int camIndex)
{
    QString strGainTime=QString::number(newValue);
    this->gainValues[camIndex]->setText(strGainTime);
}



