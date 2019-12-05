#include <QMessageBox>
#include <QThread>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <string>
#include <sstream>
#include <QString>
#include <algorithm>
#include <QKeyEvent>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    for (int i = 0; i < this->numTX2Cameras; i++) {
        this->TX2CameraThreads.push_back(new QThread());
        this->TX2Cameras.push_back(new Camera(i));
        this->TX2Cameras[i]->moveToThread(this->TX2CameraThreads[i]);
    }
    this->camerasRunning = false;
    this->camerasFinished = 0;

    ui->setupUi(this);
    this->assignLabels();
    this->connectStart();
    this->setupUiLayout();

}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::connectStart() {
    connect(ui->ctsModeStartButton, SIGNAL(clicked()), this, SLOT(startCamerasCts()));
}

void MainWindow::startCamerasCts() {

    if (!camerasRunning) {
        for (int i = 0; i < this->numTX2Cameras; i++) {

            connect(this->TX2CameraThreads[i], SIGNAL(started()),        this->TX2Cameras[i], SLOT(startSession()));
            connect(this,                      SIGNAL(restartCapture()), this->TX2Cameras[i], SLOT(restartSession()));
            connect(ui->exitButton,            SIGNAL(clicked()),        this->TX2Cameras[i], SLOT(endSession()));

            // Handle worker finishing connections
            // Worker finished signal will call quit() on the thread
            // This will end the thread's event loop and initiate the thread's finished signal
            connect(this->TX2Cameras[i],       SIGNAL(destroyed()), this,                      SLOT(closeUi()));
            connect(this->TX2Cameras[i],       SIGNAL(destroyed()), this,                      SLOT(debugApp()));
            connect(this->TX2Cameras[i],       SIGNAL(finished()),  this->TX2CameraThreads[i], SLOT(quit()));
            connect(this->TX2Cameras[i],       SIGNAL(finished()),  this->TX2Cameras[i],       SLOT(deleteLater()));
            connect(this->TX2CameraThreads[i], SIGNAL(finished()),  this->TX2CameraThreads[i], SLOT(deleteLater()));

            this->TX2CameraThreads[i]->start();
        }
    } else {
        cout << "Restarting Capture" << endl;
        emit restartCapture();
    }

    this->camerasRunning = true;
    ui->stopButton->setEnabled(true);
    ui->exitButton->setEnabled(false);
    ui->ctsModeStartButton->setEnabled(false);
}

void MainWindow::debugApp() {
    cout << "Finished" << endl;
}

void MainWindow::stopAllRequest() {
    for (int i = 0; i < this->numTX2Cameras; i++) {
        this->TX2Cameras[i]->stopButtonPressed = true;
    }
    ui->exitButton->setEnabled(true);
    ui->ctsModeStartButton->setEnabled(true);
    ui->stopButton->setEnabled(false);

    // Pause Button was Already Depressed
    if (ui->pauseButton->isChecked()) {
        ui->pauseButton->setText("Pause");
        ui->pauseButton->setChecked(false);
    }
}

void MainWindow::closeUi() {
    this->camerasFinished++;
    if (this->camerasFinished == 3) {
        this->close();
    }
}


void MainWindow::captureAllRequest()
{
    for (int i = 0; i < this->numTX2Cameras; i++) {
        this->TX2Cameras[i]->captureButtonPressed = true;
    }
}

void MainWindow::pauseAllRequest(bool clicked) {
    for (int i = 0; i < this->numTX2Cameras; i++) {
        this->TX2Cameras[i]->pauseButtonPressed = clicked;
    }
    if (clicked) {
        ui->pauseButton->setText("Resume");
    }
    else {
        cout << "resume button pressed" << endl;
        ui->pauseButton->setText("Pause");
    }
}


void MainWindow::setupUiLayout() {

    // Buttons
    ui->pauseButton->setCheckable(true);
    ui->exitButton->setEnabled(true);
    ui->ctsModeStartButton->setEnabled(true);

    connect(ui->pauseButton,   SIGNAL(clicked(bool)),this,                SLOT(pauseAllRequest(bool)));
    connect(ui->captureButton, SIGNAL(clicked()),    this,                SLOT(captureAllRequest()));
    connect(ui->stopButton,    SIGNAL(clicked()),    this,                SLOT(stopAllRequest()));

    for (int i = 0; i < this->numTX2Cameras; i++) {
        connect(this->TX2Cameras[i], SIGNAL(requestFrameSettings(int)), this, SLOT(setupFrameSettings(int)));

        // Sliders
        connect(this->exposureSliders[i], SIGNAL(valueChanged(int)), this, SLOT(setExposure(int)));
        connect(this->gainSliders[i],     SIGNAL(valueChanged(int)), this, SLOT(setGain(int)));

        this->exposureSliders[i]->setRange(30,1000);
        this->exposureSliders[i]->setValue(50);
        this->exposure = 50000;
        this->gainSliders[i]->setRange(1,250);
        this->gainSliders[i]->setValue(50);
        this->gain = 50;

        // Display Data
        connect(this->TX2Cameras[i], SIGNAL(returnQDefectImage(QImage, int)),     this, SLOT(displayQDefectImage(QImage, int)));
        connect(this->TX2Cameras[i], SIGNAL(returnQPrevDefectImage(QImage, int)), this, SLOT(displayQPrevDefectImage(QImage, int)));
        connect(this->TX2Cameras[i], SIGNAL(returnRes(int, int)),                 this, SLOT(displayRes(int, int)));
        connect(this->TX2Cameras[i], SIGNAL(returnFrameRate(double, int)),        this, SLOT(displayFrameRate(double, int)));
        connect(this->TX2Cameras[i], SIGNAL(returnCurrFrameRate(double, int)),    this, SLOT(displayCurrFrameRate(double, int)));

        //        //UI Trigger
        //        connect(ui->triggerButton, &PushButton::clicked, this->TX2Cameras[i].get(), SIGNAL(frameRequest);

        //        //Hardware Trigger
        //        connect(this->trigger.get(), &Trigger::captureRequest, this->TX2Cameras[i].get(), SIGNAL(frameRequest);
        //        connect(this->TX2Cameras[i].get(), SIGNAL(returnFrameFinished, this->trigger.get(), &Trigger::captureComplete);


    }
    //    connect(ui->stopButton,SIGNAL(clicked()), this->trigger, &Trigger::stopRequest);
    //    connect(ui->pauseButton,SIGNAL(clicked()), this->trigger, &Trigger::pauseRequest);

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

void MainWindow::setGain(int newGain) {
    this->gain = newGain;
}


void MainWindow::setExposure(int newExposure) {
    this->exposure = newExposure*1000;
}

void MainWindow::setupFrameSettings(int camIndex) {

    this->TX2Cameras[camIndex]->gain = this->gain;
    this->TX2Cameras[camIndex]->exposure = this->exposure;

    QString strExpTime=QString::number(this->exposure);
    this->exposureValues[camIndex]->setText(strExpTime+" µs");

    QString strGainTime=QString::number(this->gain);
    this->gainValues[camIndex]->setText(strGainTime);
}

void MainWindow::displayQDefectImage(QImage img_temp, int camIndex) {
    image=img_temp;
    QMatrix rm;
    rm.rotate(0);
    this->defectImages[camIndex]->setPixmap(QPixmap::fromImage(image).transformed(rm).scaled(this->defectImages[camIndex]->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::displayQPrevDefectImage(QImage img_temp, int camIndex) {
    image=img_temp;
    QMatrix rm;
    rm.rotate(0);
    this->prevDefectImages[camIndex]->setPixmap(QPixmap::fromImage(image).transformed(rm).scaled(this->defectImages[camIndex]->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::displayRes(int sensorRes, int camIndex) {
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

void MainWindow::assignLabels() {
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
}




