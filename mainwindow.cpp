#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ArgusCamera.h"
#include <iostream>
#include <string>
#include <sstream>
#include <QString>
#include <algorithm>
//<linux/delay.h>
#include <QKeyEvent>



MainWindow::MainWindow(QWidget *parent) :

    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //Initialize GPIO

    ui->setupUi(this);

    ///CAM1 UI INITIALIZATION

    //SLIDER
    ui->ExposureTimeSlider->setRange(30,40000); //do not hard code this
    ui->ExposureTimeSlider->setValue(5000);
    //ui->FocusSlider->setRange(2,1499999);
    ui->GainSlider->setRange(10.0,2500.0); //do not hard code this
    ui->GainSlider->setValue(10.0);
    //BUTTON
    ui->pauseButton->setCheckable(true);
    ui->colourInitButton->setCheckable(true);
    ui->triggerModeButton->setCheckable(true);
    //RADIO
    ui->radioOriginal->setChecked(true);
    //SPINBOX
    ui->sensorModespinBox->setMaximum(2);

    ///THREAD INITIALIZATION
    ArgusCamera1 = new MainCamera(0);
    ArgusCamera2 = new ArgusCamera(1);
    triggermode1 = new triggerMode();


    ///MAIN CAMERA: CAM1

    //SLIDER
    connect(ui->ExposureTimeSlider,SIGNAL(valueChanged(int)),ArgusCamera1,SLOT(set_Exposure(int)));
    connect(ui->FocusSlider,SIGNAL(valueChanged(int)),ArgusCamera1,SLOT(set_Focus(int)));
    connect(ui->GainSlider, SIGNAL(valueChanged(int)),this, SLOT(notifyGainChanged(int)));
    connect(this, SIGNAL(floatValueChanged(float)),ArgusCamera1, SLOT(set_Gain(float)));
    //SPINBOX
    connect(ui->sensorModespinBox, SIGNAL(valueChanged(int)),ArgusCamera1, SLOT(set_sensorMode(int)));
    //BUTTON
    connect(ui->pauseButton, SIGNAL(clicked(bool)), ArgusCamera1, SLOT(preparePause(bool)));
    connect(ui->stopButton, SIGNAL(clicked(bool)), ArgusCamera1, SLOT(prepareStop(bool)));
    connect(ui->stopButton, SIGNAL(clicked(bool)), ArgusCamera2, SLOT(prepareStop(bool)));
    connect(ui->colourInitButton, SIGNAL(clicked(bool)), ArgusCamera1, SLOT(set_colourAnalysis(bool)));
    //RADIO
    connect(ui->radioOriginal,SIGNAL(clicked(bool)),ArgusCamera1, SLOT(set_DisplayOriginal(bool)));
    connect(ui->radioFloodFill,SIGNAL(clicked(bool)),ArgusCamera1, SLOT(set_DisplayFloodFill(bool)));
    connect(ui->radioThreshold,SIGNAL(clicked(bool)),ArgusCamera1, SLOT(set_DisplayThreshold(bool)));
    connect(ui->radioGray,SIGNAL(clicked(bool)),ArgusCamera1, SLOT(set_DisplayGray(bool)));

    //***INCOMPLETE*** SENSOR MODE APPLY BUTTON
    //connect(ui->sensorModeApplyButton,SIGNAL(clicked(bool)),ArgusCamera1, SLOT(set_sensorMode(int)));
    //connect(ui->sensorModeApplyButton, SIGNAL(clicked(bool)), ArgusCamera1, SLOT(prepareSensorModeChange(bool)));
    //connect(ui->sensorModeApplyButton, SIGNAL(clicked(bool)), ArgusCamera2, SLOT(prepareSensorModeChange(bool)));
    //connect(ArgusCamera1,&ArgusCamera::return_SessionEnding,this,&MainWindow::StartSession);

    //UI DISPLAY VALUES
    connect(ArgusCamera1,&MainCamera::return_Resolution,this,&MainWindow::get_Resolu);
    connect(ArgusCamera1,&MainCamera::return_FrameRate,this,&MainWindow::get_FrameRate);
    connect(ArgusCamera1,&MainCamera::return_CurrFrameRate,this,&MainWindow::get_CurrFrameRate);
    connect(ArgusCamera1,&MainCamera::return_QImage,this,&MainWindow::get_QImage);
    connect(ArgusCamera1,&MainCamera::return_DefectImage,this,&MainWindow::get_DefectImage);
    connect(ArgusCamera1,&MainCamera::return_colourL,this,&MainWindow::get_colourL);
    connect(ArgusCamera1,&MainCamera::return_colourA,this,&MainWindow::get_colourA);
    connect(ArgusCamera1,&MainCamera::return_colourB,this,&MainWindow::get_colourB);


    //or just pass an array
    connect(ArgusCamera1,&MainCamera::return_colourBl,this,&MainWindow::get_colourBl);
    connect(ArgusCamera1,&MainCamera::return_colourG,this,&MainWindow::get_colourG);
    connect(ArgusCamera1,&MainCamera::return_colourR,this,&MainWindow::get_colourR);


    connect(ui->captureButton,SIGNAL(clicked(bool)), ArgusCamera1,SLOT(captureJPEG(bool)));
    connect(ui->main_stopButtonCAM1, SIGNAL(clicked(bool)), ArgusCamera1, SLOT(prepareStop(bool)));



///CAM2 PAGE

    ui->ExposureTimeSlider_2->setRange(30,40000); //do not hard code this
    ui->ExposureTimeSlider_2->setValue(5000);

    ui->GainSlider_2->setRange(10.0,2500.0); //do not hard code this
    ui->GainSlider_2->setValue(10.0);

    connect(ArgusCamera2,&ArgusCamera::return_QImage,this,&MainWindow::get_QImageCAM2);
    connect(ArgusCamera2,&ArgusCamera::return_Resolution,this,&MainWindow::get_ResoluCAM2);
    connect(ArgusCamera2,&ArgusCamera::return_FrameRate,this,&MainWindow::get_FrameRateCAM2);
    connect(ArgusCamera2,&ArgusCamera::return_CurrFrameRate,this,&MainWindow::get_CurrFrameRateCAM2);

    connect(ui->ExposureTimeSlider_2,SIGNAL(valueChanged(int)),ArgusCamera2,SLOT(set_Exposure(int)));
    connect(ui->GainSlider_2, SIGNAL(valueChanged(int)),this, SLOT(notifyGainChangedCAM2(int)));
    connect(this, SIGNAL(floatValueChangedCAM2(float)), ArgusCamera2, SLOT(set_Gain(float)));

    connect(ui->pauseButton, SIGNAL(clicked(bool)), ArgusCamera2, SLOT(preparePause(bool)));
    connect(ui->stopButton, SIGNAL(clicked(bool)), triggermode1, SLOT(prepareStop(bool)));
    connect(ui->main_stopButtonCAM2, SIGNAL(clicked(bool)), ArgusCamera2, SLOT(prepareStop(bool)));
    connect(ui->sensorModespinBox_2, SIGNAL(valueChanged(int)),ArgusCamera2, SLOT(set_sensorMode(int)));


    //    //connect(ui->FocusSlider_2,SIGNAL(valueChanged(int)),ArgusCamera2,SLOT(set_Focus(int)));

    //    connect(ui->stopButton, SIGNAL(clicked(bool)), ArgusCamera2, SLOT(prepareStop(bool)));
        //NOT SURE YET
    //    //connect(ui->colourInitButton, SIGNAL(clicked(bool)), ArgusCamera1, SLOT(set_colourAnalysis(bool)));
    //    //connect(ui->radioOriginal_2,SIGNAL(clicked(bool)),ArgusCamera2, SLOT(set_DisplayOriginal(bool)));
    //    //connect(ui->radioFloodFill_2,SIGNAL(clicked(bool)),ArgusCamera2, SLOT(set_DisplayFloodFill(bool)));
    //    //connect(ui->radioThreshold_2,SIGNAL(clicked(bool)),ArgusCamera2, SLOT(set_DisplayThreshold(bool)));
    //    //connect(ui->radioGray_2,SIGNAL(clicked(bool)),ArgusCamera2, SLOT(set_DisplayGray(bool)));


}


MainWindow::~MainWindow()
{
    delete ui;
}




////////////////////////////////////////////////////////////////////
///PUSH BUTTONS
////////////////////////////////////////////////////////////////////


void MainWindow::on_startButton_clicked()
{
    ArgusCamera1->start();
    ArgusCamera2->start();
}

///***INCOMPLETE***

void MainWindow::StartSession(bool)
{

}

///***INCORRECT USE***
void MainWindow::on_exitButton_clicked()
{
    ArgusCamera2->quit();
}


void MainWindow::on_colourInitButton_clicked(bool checked)
{
    if (checked){
        colourButtonPressed = true; //Dont need from mainWindow
    }
    else {
        colourButtonPressed = false;
    }
}

void MainWindow::on_pauseButton_clicked(bool checked) //this is like a toggle
{
    if (checked){
        pauseButtonPressed = true;
        ui->pauseButton->setText("Resume");
    }
    else {
        pauseButtonPressed = false;
        ui->pauseButton->setText("Pause");
    }
}

///***INCOMPLETE***
void MainWindow::on_captureButton_clicked(bool checked)
{
    cout<<"capture image" <<endl;
}

void MainWindow::on_stopButton_clicked()
{
        stopButtonPressed = true;


}


/////////////////////////////////////////////////////////////////////
///SLIDER
/// ////////////////////////////////////////////////////////////////

void MainWindow::on_ExposureTimeSlider_valueChanged(int newValue)
{
    QString strExpTime=QString::number(newValue);
    ui->label->setText(strExpTime+" µs");
}

void MainWindow::on_GainSlider_valueChanged(int value)
{
    float floatValue = value/10.0;
    emit floatValueChanged(floatValue);

    QString strGainTime=QString::number(floatValue);
    ui->labelGain->setText(strGainTime);
}

void MainWindow::on_FocusSlider_valueChanged(int newValue)
{
    QString strExpTime=QString::number(newValue);
    ui->labelFrameDuration->setText(strExpTime+" µs");
}

///INCOMPLETE
int MainWindow::get_minExposure(int newValue)
{
    return newValue;
}


///////////////////////////////////////////////////////////////////////////
///DISPLAY VALUE (SIGNALS)
///////////////////////////////////////////////////////////////////////////

//Gain

void MainWindow::notifyGainChanged(int value){
    float floatValue = value/10.0;
    emit floatValueChanged(floatValue);
}

void MainWindow::notifyGainChangedCAM2(int value){
    float floatValue = value/10.0;

    cout << "CAM 2 slider changing gain " << floatValue << endl;
    emit floatValueChangedCAM2(floatValue);
}

//Frame Rate

void MainWindow::get_FrameRate(double dFrameRate)
{
    QString strFrameRate=QString::number(dFrameRate)+"  fps";
    ui->labelFrameRate->setText(strFrameRate);
}

void MainWindow::get_CurrFrameRate(double dFrameRate)
{
    QString strFrameRate=QString::number(dFrameRate)+"  fps";
    ui->labelCurrFrameRate->setText(strFrameRate);
}

//Colour

void MainWindow::get_colourL(double m_LAB)
{
    QString str;
    if(m_LAB==0){
        str = "L:  - ";
    }
    else{
        str="L: "+ QString::number(m_LAB);
    }
    ui->labelColourL->setText(str);
}

void MainWindow::get_colourA(double m_LAB)
{
    QString str;
    if(m_LAB==0){
        str = "A:  - ";
    }
    else{
        str="A: "+ QString::number(m_LAB);
    }
    ui->labelColourA->setText(str);
}

void MainWindow::get_colourB(double m_LAB)
{
    QString str;
    if(m_LAB==0){
        str = "B:  - ";
    }
    else{
        str="B: "+ QString::number(m_LAB);
    }
    ui->labelColourB->setText(str);
}







void MainWindow::get_colourBl(double m_LAB)
{

//    QString str="B: "+ QString::number(m_LAB);
//    ui->colourDisplay->setText(str);
    BGR[0]=m_LAB;

}

void MainWindow::get_colourG(double m_LAB)
{
    BGR[1]=m_LAB;


//    QString str="G: "+ QString::number(m_LAB);
//    ui->colourDisplay->setText(str);

}

void MainWindow::get_colourR(double m_LAB)
{
    BGR[2]=m_LAB;
    emit get_colourDisplay();

}


//Resolution

void MainWindow::get_Resolu(int sensorResolution)
{
    //QString strExpTime=QString::fromStdString("get res");
    QString strExpTime=QString::number(sensorResolution)+" p";
    ui->labelResolution->setText(strExpTime);
}

//Image Display

void MainWindow::get_QImage(QImage img_temp)
{
    image=img_temp;
    QMatrix rm;
    rm.rotate(0);
    ui->QimageLabel->setPixmap(QPixmap::fromImage(image).transformed(rm).scaled(ui->QimageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}


void MainWindow::get_colourDisplay() //
{
    ui->colourDisplay->setAutoFillBackground(true);

    QString str;
    str = "QLabel { background-color: rgb(" + QString::number(BGR[2]) + "," + QString::number(BGR[1]) + "," + QString::number(BGR[0])+") }";
    ui->colourDisplay->setStyleSheet(str);

}



////////////////////////////////////////////////////////////////
///CAM2
////////////////////////////////////////////////////////////////


//Slider

void MainWindow::on_ExposureTimeSlider_2_valueChanged(int newValue)
{
    QString strExpTime=QString::number(newValue);
    ui->label_2->setText(strExpTime+ " µs");
}

void MainWindow::on_GainSlider_2_valueChanged(int value)
{
    float floatValue = value/10.0;
    emit floatValueChangedCAM2(floatValue);

    QString strGainTime=QString::number(floatValue);
    ui->labelGain_2->setText(strGainTime);
    cout << "GainSlider Value Change" << endl;
}

void MainWindow::on_FocusSlider_valueChanged_2(int newValue)
{
    QString strExpTime=QString::number(newValue);
    ui->labelFrameDuration_2->setText(strExpTime+" µs");
}




//Display

void MainWindow::get_QImageCAM2(QImage img_temp)
{
    image=img_temp;
    QMatrix rm;
    rm.rotate(0);
    ui->Qimage2Label->setPixmap(QPixmap::fromImage(image).transformed(rm).scaled(ui->Qimage2Label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::get_DefectImage(QImage img_temp)
{
    image=img_temp;
    QMatrix rm;
    rm.rotate(0);
    ui->QimageDefect->setPixmap(QPixmap::fromImage(image).transformed(rm).scaled(ui->QimageDefect->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::get_ResoluCAM2(int sensorResolution)
{
    QString strExpTime=QString::number(sensorResolution)+" p";
    ui->labelResolution_2->setText(strExpTime);
}

void MainWindow::get_FrameRateCAM2(double dFrameRate)
{
    QString strFrameRate=QString::number(dFrameRate)+"  fps";
    ui->labelFrameRate_2->setText(strFrameRate);
}

void MainWindow::get_CurrFrameRateCAM2(double dFrameRate)
{
    QString strFrameRate=QString::number(dFrameRate)+"  fps";
    ui->labelCurrFrameRate_2->setText(strFrameRate);
}



///SENSOR MODE
void MainWindow::on_sensorModespinBox_valueChanged(int arg1)
{
    QString strExpTime=QString::number(arg1);
    //ui->labelResolution->setText(strExpTime);
}


void MainWindow::on_resetButton_clicked()
{
    ui->ExposureTimeSlider->setValue(3500);
    ui->GainSlider->setValue(170.0);
    ui->sensorModespinBox->setValue(2);

}


void MainWindow::on_sensorModeApplyButton_clicked()
{
//    ui->stopButton->click();

//    ArgusCamera1 = new ArgusCamera(0); //start running camera on start pressed
//    ArgusCamera2 = new ArgusCamera(1);

//   ui->startButton->click();

}

void MainWindow::on_sensorModeApplyButton_pressed()
{
    ui->stopButton->clicked();
    ui->startButton->clicked();
}

void MainWindow::on_sensorModeApplyButton_released()
{
    ui->startButton->clicked();
}

void MainWindow::on_sensorModeApplyButton_2_pressed()
{
    ui->stopButton->clicked();
}

void MainWindow::on_sensorModeApplyButton_2_released()
{
    ui->startButton->clicked();
}

////////////////////////////////////////////////
///***INCOMPLETE***
////////////////////////////////////////////////
void MainWindow::on_triggerModeButton_clicked(bool checked)
{

}

void MainWindow::on_triggerModeButton_clicked()
{
    //if DetectValue is high

    /*CHECK FOR GPIO INTERRUPT*/
    unsigned int Detectvalue=high;
    gpioGetValue(ButtonSigPin,&Detectvalue);

    if (Detectvalue==high) {//***Add condition on if picture not taken
//        cout<< "Detectvalue==high    :" << Detectvalue <<endl;
//        string savepath = "/home/nvidia/Desktop/capture" + std::to_string(frameCaptureLoop) + ".png";
//        cv::imwrite(savepath, tej);

        triggermode1->start();

     }
    else {
        cout<< "Detectvalue==low    :" << Detectvalue <<endl; //Take Picture
     }

//    mSN = new QSocketNotifier; // defined in .h
//    QFile file("/dev/testDriver");
//    if(file.open(QFile::ReadOnly)) {
//      QSocketNotifier mSN(file.handle(), , QSocketNotifier::Read);
//      mSN.setEnabled(true);
//      connect(mSN, SIGNAL(activated(int)), &this, SLOT(readyRead()));
//    }

}



///MAIN PAGE

void MainWindow::on_main_startButtonCAM1_clicked()
{
    ArgusCamera1->start();
}

void MainWindow::on_main_startButtonCAM2_clicked()
{
    ArgusCamera2->start();
}

void MainWindow::on_main_stopButtonCAM1_clicked()
{
    stopButtonPressed = true;
}

void MainWindow::on_main_stopButtonCAM2_clicked()
{
    stopButtonPressed = true;
}
