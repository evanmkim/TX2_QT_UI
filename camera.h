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
#include <chrono>
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

    Camera(int camDeviceIndex = 0) : cameraDeviceIndex(camDeviceIndex){}

    bool initCam();

    // boolean flags set by MainWindow on_clicked methods
    bool stopButtonPressed = false;
    bool captureButtonPressed = false;
    bool pauseButtonPressed = false;

    // Continuous Mode: 0
    // Triggered Mode: 1
    int captureMode = 0;

protected:
    void run();

private:

    // Exposure Settings
    int curExposure=0;
    int curGain=0;

    // Session Info
    Argus::Status status;
    std::vector<CameraDevice *> cameraDevices;
    int frameCaptureCount=0;
    int cameraDeviceIndex=0;

    // Session Frame Rate
    const uint64_t ONE_SECOND = 1000000000;
    float previousTimeStamp=0.0;
    float sensorTimeStamp=0.0;

    int frameCaptureLoop;

    chrono::high_resolution_clock::time_point startTime;
    chrono::high_resolution_clock::time_point finishTime;

    uint64_t previousFrameNum=0;

    // Session Interfaces
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

    // Cts Session Interfaces
    IEventProvider *iEventProvider = nullptr;

    Argus::UniqueObj<Argus::EventQueue> queue;
    IEventQueue *iQueue = nullptr;

    ISourceSettings *iSourceSettings = nullptr;
    ICameraProperties *iCameraProperties = nullptr;
    ISensorMode *iSensorMode = nullptr;

    IAutoControlSettings *iAutoControlSettings = nullptr;

    const ICaptureMetadata *iMetadata = nullptr;

    Argus::UniqueObj<EGLStream::Frame> frame;
    EGLStream::IFrame  *iFrame = nullptr;

    // CV Processing
    int DisplayIndex=1;
    Mat imShow[4][10]; //2D Array that saves frames in an array to display
    ArgusSamples::EGLDisplayHolder g_display;

    // Provides exclusive write access to frameFinished
    QMutex mutex;
    static int frameFinished;

    bool defectFound = false;

    bool runCts();
    void endCapture();

signals:
    //void returnQImage(QImage, int);
    void returnQDefectImage(QImage, int);
    void returnQPrevDefectImage(QImage, int);
    void returnRes(int, int);
    void returnFrameFinished(bool);
    void returnFrameRate(double, int);
    void returnCurrFrameRate(double, int);
    void returnExposureVal(int, int);
    void returnGainVal(int, int);

public slots:
    void stopRequest();
    void pauseRequest(bool);
    bool frameRequest();
    void saveRequest();
    void setExposure(int);
    void setGain(int);
};


#endif // Camera_H
