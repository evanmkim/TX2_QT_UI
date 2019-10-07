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

public:
    explicit Camera(QObject *parent = 0);

    Camera(int camDeviceIndex = 0) : cameraDeviceIndex(camDeviceIndex){

    }

    bool initCam();
    bool stopButtonPressed = false;

protected:
    void run();

private:
    Argus::Status status;

    std::vector<CameraDevice *> cameraDevices;

    Argus::UniqueObj<Argus::CameraProvider> cameraProvider;
    ICameraProvider *iCameraProvider = nullptr;

    Argus::UniqueObj<Argus::CaptureSession> captureSession;
    ICaptureSession *iSession = nullptr;

    Argus::UniqueObj<Argus::OutputStreamSettings> streamSettings;
    IOutputStreamSettings *iStreamSettings = nullptr;

    Argus::UniqueObj<Argus::OutputStream> stream;
    IStream *iStream = nullptr;

    Argus::UniqueObj<EGLStream::FrameConsumer> consumer;
    EGLStream::IFrameConsumer *iFrameConsumer = nullptr;

    Argus::UniqueObj<Argus::Request> request;
    Argus::IRequest *iRequest = nullptr;

    int frameCaptureCount=0;
    int cameraDeviceIndex=0;
    int DisplayIndex=1;

    Mat imShow[4][10]; //2D Array that saves frames in an array to display
    ArgusSamples::EGLDisplayHolder g_display;

signals:
    void return_QImage1(QImage);
    void return_DefectImage1(QImage);
    void return_QImage2(QImage);
    void return_DefectImage2(QImage);
    void return_QImage3(QImage);
    void return_DefectImage3(QImage);

public slots:
    void stopRequest(bool);
    bool triggerRequest(bool);
};


#endif // Camera_H
