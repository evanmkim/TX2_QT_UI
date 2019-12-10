#ifndef CAMERA_H
#define CAMERA_H
#define QT_THREAD_SUPPORT
#include <qmutex.h>
#include <QWaitCondition>
#include <QTimer>
#include <QString>
#include <QThread>
#include <QAbstractEventDispatcher>
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

class Camera : public QObject
{
    Q_OBJECT

// Public members are modified at runtime by the main GUI thread
public:
    Camera(int camDeviceIndex);

    // boolean flags set by MainWindow on_clicked methods
    bool stopButtonPressed;
    bool captureButtonPressed;
    bool pauseButtonPressed;

    bool triggered;

    // Continuous Mode: 0
    // Triggered Mode: 1
    int captureMode;

    // Frame Settings
    int displayIndex;
    int exposure;
    int gain;

private:

    // Session Info
    Argus::Status status;
    std::vector<CameraDevice *> cameraDevices;
    int frameCaptureCount;
    int cameraDeviceIndex;

    // Session Frame Rate
    const uint64_t ONE_SECOND = 1000000000;
    float previousTimeStamp;
    float sensorTimeStamp;

    chrono::high_resolution_clock::time_point startTime;
    chrono::high_resolution_clock::time_point finishTime;

    uint64_t previousFrameNum=0;

    // Session Interfaces
    Argus::UniqueObj<Argus::CameraProvider> cameraProvider;
    ICameraProvider *iCameraProvider;

    Argus::UniqueObj<Argus::CaptureSession> captureSession;
    ICaptureSession *iSession;

    Argus::UniqueObj<Argus::OutputStreamSettings> streamSettings;
    IOutputStreamSettings *iStreamSettings;

    Argus::UniqueObj<Argus::OutputStream> stream;
    IStream *iStream;

    Argus::UniqueObj<EGLStream::FrameConsumer> consumer;
    EGLStream::IFrameConsumer *iFrameConsumer;

    Argus::UniqueObj<Argus::Request> request;
    Argus::IRequest *iRequest;

    // Cts Session Interfaces
    IEventProvider *iEventProvider;

    Argus::UniqueObj<Argus::EventQueue> queue;
    IEventQueue *iQueue;

    ISourceSettings *iSourceSettings;
    ICameraProperties *iCameraProperties;
    ISensorMode *iSensorMode;

    IAutoControlSettings *iAutoControlSettings;

    IDenoiseSettings *iDenoiseSettings;

    const ICaptureMetadata *iMetadata;

    Argus::UniqueObj<EGLStream::Frame> frame;
    EGLStream::IFrame  *iFrame;

    // CV Processing
    Mat imShow[3][10]; //2D Array that saves frames in an array to display
    ArgusSamples::EGLDisplayHolder g_display;

    // Provides exclusive write access to frameFinished
//    QMutex *mutex;
//    int *frameFinished;

    bool defectFound = false;
    bool runCts();
    bool frameRequest();

    bool restarted;

signals:

    void finished();
    void requestFrameSettings(int);

    //void returnQImage(QImage, int);
    void returnQDefectImage(QImage, int);
    void returnQPrevDefectImage(QImage, int);
    void returnRes(int, int);
    void returnFrameFinished();
    void returnFrameRate(double, int);
    void returnCurrFrameRate(double, int);
    void returnFrameCount(int,int);

public slots:

    bool startSession();
    bool restartSession();
    void endSession();

};


#endif // Camera_H
