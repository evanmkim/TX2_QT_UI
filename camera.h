#ifndef CAMERA_H
#define CAMERA_H
#define QT_THREAD_SUPPORT
#include <qmutex.h>
#include <QWaitCondition>
#include <QTimer>
#include <QString>
#include <QThread>
#include <QObject>
#include <QImage>
#include <QDebug>
#include <iostream>
#include <time.h>
#include <Argus/Argus.h>
#include <EGL/egl.h>
#include <EGLStream/EGLStream.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <Argus/Ext/BayerAverageMap.h>
#include "utils/Options.h"
#include <algorithm>
#include <math.h>
#include "utils/PreviewConsumer.h"
#include "utils/Error.h"
#include "SimpleCV.h"
#include "jetsonGPIO.h"
#undef Bool

using namespace std;
using namespace Argus;

#define EXIT_IF_TRUE(val,msg)   \
{if ((val)) {printf("%s\n",msg); return EXIT_FAILURE;}}
#define EXIT_IF_NULL(val,msg)   \
{if (!val) {printf("%s\n",msg); return EXIT_FAILURE;}}
#define EXIT_IF_NOT_OK(val,msg) \
{if (val!=Argus::STATUS_OK) {printf("%s\n",msg); return EXIT_FAILURE;}}

using namespace cv;
using namespace std;

class Camera : public QThread
{
    Q_OBJECT

private:
    QMutex sync;
    QWaitCondition pauseCond;

public:
    explicit Camera(QObject *parent = 0);

    Camera(int camDeviceIndex = 0, int set_count=0) {
        count=set_count;
        cameraDeviceIndex = camDeviceIndex;
    }

    void show_cam();//all general functions
    bool initCam();
    void putFrameInBuffer(Mat &f);


protected:
    void run();

private:

    int count;
    QTimer *timer;
    ArgusSamples::EGLDisplayHolder g_display;
    int cameraDeviceIndex;
    int DisplayIndex=1;

    //Threading
    int idx;
    int pos;
    int buffLen=50;
    Mat frameBuffer[50];
    Mat imShow[4][10]; //2D Array that saves frames in an array to display

    bool stopButtonPressed = false;
    bool triggerButtonPressed = false;
    std::vector <double> LAB;

signals:
    void return_QImage1(QImage);
    void return_DefectImage1(QImage);
    void return_QImage2(QImage);
    void return_DefectImage2(QImage);
    void return_QImage3(QImage);
    void return_DefectImage3(QImage);

public slots:
    void stopRequest(bool);
    void triggerRequest(bool);

};


#endif // Camera_H
