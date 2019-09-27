#ifndef ARGUSCAMERA
#define ARGUSCAMERA
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
//using namespace std;
//using namespace Argus;

class ArgusCamera : public QThread
{
    Q_OBJECT

private:
    QMutex sync;
    QWaitCondition pauseCond;
    bool pause;

public:
    explicit ArgusCamera(QObject *parent = 0);

    ArgusCamera(int camDeviceIndex = 0, int set_count=0): pause(false){
        count=set_count;
        cameraDeviceIndex = camDeviceIndex;
    }


    bool EncoderGPIO(bool);

    struct ExecuteOptions
    {
        uint32_t cameraIndex;
        uint32_t useAverageMap;
    };

    void show_cam();//all general functions
    bool initCAM();
    void resume();
    void paused();
    void putFrameInBuffer(Mat &f);


protected:
    void run();

private:

//Unused Variables
    int count;
    int minExposure;
    int maxExposure;
    QTimer *timer;
    ArgusSamples::EGLDisplayHolder g_display;


//Default Values
    const int DEFAULT_EXPOSURE_TIME = 5000000;//camera default exposure
    const int CAPTURE_COUNT = 700000;
    int cameraDeviceIndex;
    int DisplayIndex=1;
    jetsonTX1GPIONumber ButtonSigPin = gpio184;


//Threading
    int idx;
    int pos;
    int buffLen=50;
    Mat frameBuffer[50];
    Mat imShow[4][10]; //2D Array that saves frames in an array to display

//    virtual bool threadInitialize();
//    virtual bool threadExecute();
//    virtual bool threadShutdown();


signals:
    void return_minExposure(int); //Not used
    void return_maxExposure(int);
    void return_FrameRate(double);
    void return_CurrFrameRate(double);
    void return_QImage(QImage);
    void return_DefectImage(QImage);
    void return_colourL(double);
    void return_colourA(double);
    void return_colourB(double);
    void return_colourBl(double);
    void return_colourG(double);
    void return_colourR(double);
    void return_Resolution(int);

    void return_SessionEnding(bool);
    //void return_sensorMode(int);

public slots:
    void set_sensorMode(int);
    void set_Focus(int);
    void set_Exposure(int);
    void set_Gain(float);
//    void set_GainCAM2(float);
    void set_colourAnalysis(bool);
    void preparePause(bool);
    void prepareStop(bool);
    void triggerRequest(bool);

    void prepareSensorModeChange(bool);

    //double getTimeMS(void);
    void captureJPEG(bool);
    void set_DisplayOriginal(bool);
    void set_DisplayFloodFill(bool);
    void set_DisplayThreshold(bool);
    void set_DisplayGray(bool);

 private:
    int curExposure=DEFAULT_EXPOSURE_TIME;
    int curFocus=2;
    //uint64_t newFocusPos = curFocusPos;
    float curGain=1;
    int sensorModeIndex=0;
    bool pauseButtonPressed = false;
    bool stopButtonPressed = false;
    bool sensorModeApplyButtonPressed = false;
    bool colourButtonPressed = false;
    bool captureButtonPressed = false;
    bool triggerButtonPressed = false;

    std::vector <double> LAB;

//    ICameraProvider *iCameraProvider;
//    std::vector<CameraDevice*> cameraDevices;
//    ICaptureSession *iSession;
//    IEventProvider *iEventProvider;
//    std::vector<EventType> eventTypes;
//    IEventQueue *iQueue;
//    IOutputStreamSettings *iStreamSettings;
//    IStream *iStream;
//    EGLStream::IFrameConsumer *iFrameConsumer;
//    IRequest *iRequest;
//    Argus::ISourceSettings *iSourceSettings;
//    UniqueObj<Request> request;
//    UniqueObj<OutputStream> stream;
//    UniqueObj<CameraProvider> cameraProvider;

};


#endif // ARGUSEXPOSURE

