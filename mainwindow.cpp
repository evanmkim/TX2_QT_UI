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
        this->TX2Cameras.push_back(new Camera(i));
    }


    ///MAIN CAMERA: CAM1
    ///
    for (int i = 0; i < this->numTX2Cameras; i++) {
        connect(ui->stopButton, SIGNAL(clicked(bool)), this->TX2Cameras[i], SLOT(stopRequest(bool)));
        connect(ui->triggerButton,SIGNAL(clicked(bool)), this->TX2Cameras[i],SLOT(triggerRequest(bool)));
    }

    connect(this->TX2Cameras[0],&Camera::return_QImage1,this,&MainWindow::display_QImage1);
    connect(this->TX2Cameras[0],&Camera::return_DefectImage1,this,&MainWindow::display_DefectImage1);
    connect(this->TX2Cameras[1],&Camera::return_QImage2,this,&MainWindow::display_QImage2);
    connect(this->TX2Cameras[1],&Camera::return_DefectImage2,this,&MainWindow::display_DefectImage2);
    connect(this->TX2Cameras[2],&Camera::return_QImage3,this,&MainWindow::display_QImage3);
    connect(this->TX2Cameras[2],&Camera::return_DefectImage3,this,&MainWindow::display_DefectImage3);

    images[0] = ui->QImageLabel1;
    images[1] = ui->QImageLabel2;
    images[2] = ui->QImageLabel3;
    defectImages[0] = ui->QDefectLabel1;
    defectImages[1] = ui->QDefectLabel2;
    defectImages[2] = ui->QDefectLabel3;
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_startButton_clicked()
{
    for (int i = 0; i < this->numTX2Cameras; i++) {
        this->TX2Cameras[i]->start();
    }
}

void MainWindow::on_exitButton_clicked()
{
    this->close();
}

void MainWindow::display_QImage1(QImage img_temp)
{
    image=img_temp;
    QMatrix rm;
    rm.rotate(0);
    this->images[0]->setPixmap(QPixmap::fromImage(image).transformed(rm).scaled(this->images[0]->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::display_DefectImage1(QImage img_temp)
{
    image=img_temp;
    QMatrix rm;
    rm.rotate(0);
    this->defectImages[0]->setPixmap(QPixmap::fromImage(image).transformed(rm).scaled(this->defectImages[0]->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::display_QImage2(QImage img_temp)
{
    image=img_temp;
    QMatrix rm;
    rm.rotate(0);
    this->images[1]->setPixmap(QPixmap::fromImage(image).transformed(rm).scaled(this->images[1]->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::display_DefectImage2(QImage img_temp)
{
    image=img_temp;
    QMatrix rm;
    rm.rotate(0);
    this->defectImages[1]->setPixmap(QPixmap::fromImage(image).transformed(rm).scaled(this->defectImages[1]->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::display_QImage3(QImage img_temp)
{
    image=img_temp;
    QMatrix rm;
    rm.rotate(0);
    this->images[2]->setPixmap(QPixmap::fromImage(image).transformed(rm).scaled(this->images[2]->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::display_DefectImage3(QImage img_temp)
{
    image=img_temp;
    QMatrix rm;
    rm.rotate(0);
    this->defectImages[2]->setPixmap(QPixmap::fromImage(image).transformed(rm).scaled(this->defectImages[2]->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::on_stopButton_clicked()
{
    this->stopButtonPressed = true;
}

