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

    this->triggerThread = new QThread();
    this->trigger = new Trigger();
    this->trigger->moveToThread(this->triggerThread);

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
    connect(ui->tgrModeStartButton, SIGNAL(clicked()), this, SLOT(startCamerasTgr()));
}

void MainWindow::startCamerasCts() {

    if (!camerasRunning) {
        for (int i = 0; i < this->numTX2Cameras; i++) {

            this->TX2Cameras[i]->captureMode = 0;

            connect(this->TX2CameraThreads[i], SIGNAL(started()),        this->TX2Cameras[i], SLOT(startSession()));
            connect(this,                      SIGNAL(restartCapture()), this->TX2Cameras[i], SLOT(restartSession()));
            connect(ui->exitButton,            SIGNAL(clicked()),        this->TX2Cameras[i], SLOT(endSession()));

            // Handle worker finishing connections
            // Worker finished signal will call quit() on the thread
            // This will end the thread's event loop and initiate the thread's finished signal
            connect(this->TX2Cameras[i],       SIGNAL(destroyed()), this,                      SLOT(closeUi()));
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

void MainWindow::startCamerasTgr() {
    if (!camerasRunning) {
        for (int i = 0; i < this->numTX2Cameras; i++) {

            this->TX2Cameras[i]->captureMode = 1;

            connect(this->TX2Cameras[i],       SIGNAL(frameFinished()), this, SLOT(captureComplete()));
            connect(this->TX2CameraThreads[i], SIGNAL(started()),        this->TX2Cameras[i], SLOT(startSession()));
            connect(this,                      SIGNAL(restartCapture()), this->TX2Cameras[i], SLOT(restartSession()));
            connect(ui->exitButton,            SIGNAL(clicked()),        this->TX2Cameras[i], SLOT(endSession()));

            // Handle worker finishing connections
            // Worker finished signal will call quit() on the thread
            // This will end the thread's event loop and initiate the thread's finished signal
            connect(this->TX2Cameras[i],       SIGNAL(destroyed()), this,                      SLOT(closeUi()));
            connect(this->TX2Cameras[i],       SIGNAL(finished()),  this->TX2CameraThreads[i], SLOT(quit()));
            connect(this->TX2Cameras[i],       SIGNAL(finished()),  this->TX2Cameras[i],       SLOT(deleteLater()));
            connect(this->TX2CameraThreads[i], SIGNAL(finished()),  this->TX2CameraThreads[i], SLOT(deleteLater()));

            this->TX2CameraThreads[i]->start();
        }
        connect(this->trigger,       SIGNAL(captureRequest()), this,                SLOT(frameRequest()));
        connect(this->triggerThread, SIGNAL(started()),        this->trigger,       SLOT(startSession()));
        connect(this->trigger,       SIGNAL(finished()),       this->triggerThread, SLOT(quit()));
        connect(this->trigger,       SIGNAL(finished()),       this->trigger,       SLOT(deleteLater()));
        connect(this->triggerThread, SIGNAL(finished()),       this->triggerThread, SLOT(deleteLater()));

        this->triggerThread->start();

    } else {
        cout << "Restarting Capture" << endl;
        emit restartCapture();
    }

    this->camerasRunning = true;
    ui->stopButton->setEnabled(true);
    ui->exitButton->setEnabled(false);
    ui->ctsModeStartButton->setEnabled(false);
}

void MainWindow::captureComplete() {
    this->trigger->frameFinished++;
}

void MainWindow::frameRequest() {
    for (int i = 0; i < this->numTX2Cameras; i++) {
        this->TX2Cameras[i]->triggered = true;
    }
}


void MainWindow::stopAllRequest() {
    for (int i = 0; i < this->numTX2Cameras; i++) {
        this->TX2Cameras[i]->stopButtonPressed = true;
    }
    this->trigger->stopButtonPressed = true;

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
    this->trigger->pauseButtonPressed = true;

    if (clicked) {
        ui->pauseButton->setText("Resume");
    }
    else {
        cout << "resume button pressed" << endl;
        ui->pauseButton->setText("Pause");
    }
}

void MainWindow::setupUiLayout() {

    // Radio Buttons
    ui->pauseButton->setCheckable(true);
    ui->exitButton->setEnabled(true);
    ui->ctsModeStartButton->setEnabled(true);

    connect(ui->pauseButton,   SIGNAL(clicked(bool)),this, SLOT(pauseAllRequest(bool)));
    connect(ui->captureButton, SIGNAL(clicked()),    this, SLOT(captureAllRequest()));
    connect(ui->stopButton,    SIGNAL(clicked()),    this, SLOT(stopAllRequest()));

    // Push Buttons
    ui->normalRadioButton->setChecked(true);

    connect(ui->normalRadioButton, SIGNAL(clicked(bool)), this, SLOT(displayNormal(bool)));
    connect(ui->gScaleRadioButton, SIGNAL(clicked(bool)), this, SLOT(displayGreyscale(bool)));
    connect(ui->fFillRadioButton,  SIGNAL(clicked(bool)), this, SLOT(displayFloodfill(bool)));
    connect(ui->tHoldRadioButton,  SIGNAL(clicked(bool)), this, SLOT(displayThreshold(bool)));

    // Drop Downs
    this->exposure = 30000;
    this->gain = 30;

    for (int i = 0; i < dropDownOptionCount; i++) {

        ui->exposureDropDown->addItem(QString::number((exposure/1000)+(i*10)), exposure+(i*10000));
        ui->gainDropDown->addItem(QString::number(gain+(i*10)), gain+(i*10));
    }

    connect(ui->exposureDropDown, SIGNAL(currentIndexChanged(int)), this, SLOT(setExposure(int)));
    connect(ui->gainDropDown,     SIGNAL(currentIndexChanged(int)), this, SLOT(setGain(int)));

    for (int i = 0; i < this->numTX2Cameras; i++) {

        connect(this->TX2Cameras[i], SIGNAL(requestFrameSettings(int)), this, SLOT(setupFrameSettings(int)));

        // Display Data
        connect(this->TX2Cameras[i], SIGNAL(returnQDefectImage(QImage, int)),     this, SLOT(displayQDefectImage(QImage, int)));
        connect(this->TX2Cameras[i], SIGNAL(returnQPrevDefectImage(QImage, int)), this, SLOT(displayQPrevDefectImage(QImage, int)));
        connect(this->TX2Cameras[i], SIGNAL(returnRes(int, int)),                 this, SLOT(displayRes(int, int)));
        connect(this->TX2Cameras[i], SIGNAL(returnFrameRate(double, int)),        this, SLOT(displayFrameRate(double, int)));
        connect(this->TX2Cameras[i], SIGNAL(returnCurrFrameRate(double, int)),    this, SLOT(displayCurrFrameRate(double, int)));
        connect(this->TX2Cameras[i], SIGNAL(returnCurrFrameRate(double, int)),    this, SLOT(displayCurrFrameRate(double, int)));
        connect(this->TX2Cameras[i], SIGNAL(returnFrameCount(int, int)),          this, SLOT(displayFrameCount(int, int)));
    }

    // Taymer Logo
    QString filename = "/home/nvidia/ExposureUIQt/Assets/TaymerLogoCropped.png";
    ui->TaymerLogo->setAlignment(Qt::AlignCenter);
    QPixmap logo;

    if (logo.load(filename)) {
        logo = logo.scaled(ui->TaymerLogo->size(),Qt::KeepAspectRatio);
        ui->TaymerLogo->setPixmap(logo);
    } else {
        cout << "Unable to load, bad filename" << endl;
    }
}

void MainWindow::displayNormal(bool checked) {
    if (checked) {
        for(int i = 0; i < this->numTX2Cameras; i++) {
            this->TX2Cameras[i]->displayIndex = 0;
        }
    }
}

void MainWindow::displayThreshold(bool checked) {
    if (checked) {
        for(int i = 0; i < this->numTX2Cameras; i++) {
            this->TX2Cameras[i]->displayIndex = 1;
        }
    }
}

void MainWindow::displayFloodfill(bool checked) {
    if (checked) {
        for(int i = 0; i < this->numTX2Cameras; i++) {
            this->TX2Cameras[i]->displayIndex = 2;
        }
    }
}

void MainWindow::displayGreyscale(bool checked) {
    if (checked) {
        for(int i = 0; i < this->numTX2Cameras; i++) {
            this->TX2Cameras[i]->displayIndex = 3;
        }
    }
}

void MainWindow::setGain(int gainIndex) {
    this->gain = ui->gainDropDown->itemData(gainIndex).toInt();
}


void MainWindow::setExposure(int exposureIndex) {
    this->exposure = (ui->exposureDropDown->itemData(exposureIndex).toInt());
}

void MainWindow::setupFrameSettings(int camIndex) {
    this->TX2Cameras[camIndex]->gain = this->gain;
    this->TX2Cameras[camIndex]->exposure = this->exposure;
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
    QString strExpTime = QString::number(sensorRes)+" p";
    this->resolutions[camIndex]->setText(strExpTime);
}

void MainWindow::displayFrameRate(double frameRate, int camIndex) {
    QString strFrameRate = QString::number(frameRate)+"  fps";
    this->frameRates[camIndex]->setText(strFrameRate);
}

void MainWindow::displayCurrFrameRate(double currFrameRate, int camIndex) {
    QString strCurrFrameRate = QString::number(currFrameRate)+"  fps";
    this->currFrameRates[camIndex]->setText(strCurrFrameRate);
}

void MainWindow::displayFrameCount(int frameCount, int camIndex) {
    QString strFrameCount = QString::number(frameCount);
    this->frameCounts[camIndex]->setText(strFrameCount);
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

    this->frameCounts.push_back(ui->labelFrameCount);
    this->frameCounts.push_back(ui->labelFrameCount_2);
    this->frameCounts.push_back(ui->labelFrameCount_3);
}




