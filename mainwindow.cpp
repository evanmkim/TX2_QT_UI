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
        //connect(ui->pauseButton, &QPushButton::clicked, this->TX2Cameras[i].get(), &Camera::pauseRequest);
        connect(ui->pauseButton, SIGNAL(clicked(bool)), this->TX2Cameras[i].get(), SLOT(pauseRequest(bool)));
        connect(ui->captureButton, &QPushButton::clicked, this->TX2Cameras[i].get(), &Camera::saveRequest);


        // UI Trigger
        //connect(ui->triggerButton, &PushButton::clicked, this->TX2Cameras[i].get(), &Camera::triggerRequest);

        // Hardware Trigger
        connect(this->trigger.get(), &Trigger::captureRequest, this->TX2Cameras[i].get(), &Camera::triggerRequest);
        connect(this->TX2Cameras[i].get(), &Camera::returnFrameFinished, this->trigger.get(), &Trigger::captureComplete);
    }
    connect(ui->stopButton, SIGNAL(clicked(bool)), this->trigger.get(), SLOT(stopRequest(bool)));
    connect(ui->pauseButton, SIGNAL(clicked(bool)), this->trigger.get(), SLOT(pauseRequest(bool)));

    connect(this->TX2Cameras[0].get(),&Camera::returnQImage1,this,&MainWindow::displayQImage1);
    connect(this->TX2Cameras[0].get(),&Camera::returnDefectImage1,this,&MainWindow::displayDefectImage1);
    connect(this->TX2Cameras[1].get(),&Camera::returnQImage2,this,&MainWindow::displayQImage2);
    connect(this->TX2Cameras[1].get(),&Camera::returnDefectImage2,this,&MainWindow::displayDefectImage2);
    connect(this->TX2Cameras[2].get(),&Camera::returnQImage3,this,&MainWindow::displayQImage3);
    connect(this->TX2Cameras[2].get(),&Camera::returnDefectImage3,this,&MainWindow::displayDefectImage3);

    //    images[0] = ui->QImageLabel1;
    //    images[1] = ui->QImageLabel2;
    //    images[2] = ui->QImageLabel3;
    defectImages[0] = ui->QImageLabel1;
    defectImages[1] = ui->QImageLabel2;
    defectImages[2] = ui->QImageLabel3;

    ui->pauseButton->setCheckable(true);
}


MainWindow::~MainWindow()
{
    delete ui;
}

// Start into continuous capture at 30 fps (for live view)
void MainWindow::on_ctsModeStartButton_clicked()
{
    this->trigger->start();
    for (int i = 0; i < this->numTX2Cameras; i++) {
        this->TX2Cameras[i]->start();
    }
}

// Start into triggered capture, hardware synchronized (for sync view)
void MainWindow::on_tgrModeStartButton_clicked()
{
    this->trigger->start();
    for (int i = 0; i < this->numTX2Cameras; i++) {
        this->TX2Cameras[i]->start();
    }
}


void MainWindow::on_exitButton_clicked()
{
    this->close();
}

// SHOULD BE ABLE TO CAPTURE CURRENT FRAME WHILE PAUSED
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

void MainWindow::displayQImage1(QImage img_temp)
{
    image=img_temp;
    QMatrix rm;
    rm.rotate(0);
    this->images[0]->setPixmap(QPixmap::fromImage(image).transformed(rm).scaled(this->images[0]->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::displayDefectImage1(QImage img_temp)
{
    image=img_temp;
    QMatrix rm;
    rm.rotate(0);
    this->defectImages[0]->setPixmap(QPixmap::fromImage(image).transformed(rm).scaled(this->defectImages[0]->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::displayQImage2(QImage img_temp)
{
    image=img_temp;
    QMatrix rm;
    rm.rotate(0);
    this->images[1]->setPixmap(QPixmap::fromImage(image).transformed(rm).scaled(this->images[1]->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::displayDefectImage2(QImage img_temp)
{
    image=img_temp;
    QMatrix rm;
    rm.rotate(0);
    this->defectImages[1]->setPixmap(QPixmap::fromImage(image).transformed(rm).scaled(this->defectImages[1]->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::displayQImage3(QImage img_temp)
{
    image=img_temp;
    QMatrix rm;
    rm.rotate(0);
    this->images[2]->setPixmap(QPixmap::fromImage(image).transformed(rm).scaled(this->images[2]->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::displayDefectImage3(QImage img_temp)
{
    image=img_temp;
    QMatrix rm;
    rm.rotate(0);
    this->defectImages[2]->setPixmap(QPixmap::fromImage(image).transformed(rm).scaled(this->defectImages[2]->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

