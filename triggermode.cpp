#include "triggermode.h"
#include "ArgusCamera.h"
#include "jetsonGPIO.h"
#include <Argus/Argus.h>
#include <EGL/egl.h>
#include <EGLStream/EGLStream.h>
#include <nvbuf_utils.h>
#include "EGLStream/NV/ImageNativeBuffer.h"
#include <chrono> //for time

using namespace Argus;
using namespace std;

#define EXIT_IF_NULL(val,msg)   \
        {if (!val) {printf("%s\n",msg); return EXIT_FAILURE;}}
#define EXIT_IF_NOT_OK(val,msg) \
        {if (val!=Argus::STATUS_OK) {printf("%s\n",msg); return EXIT_FAILURE;}}

static const uint32_t         DEFAULT_CAMERA_INDEX = 0;






triggerMode::triggerMode(QObject *parent): QThread(parent)
{



}


void triggerMode::run()
{
    //Initialize GPIO
    gpioExport(ButtonSigPin);
    msleep(1000); //Need this
    gpioSetDirection(ButtonSigPin,inputPin);
    gpioSetEdge ( ButtonSigPin, "rising");

//    timer=new QTimer();
//    timer->start(50);
//    //connect(timer,&QTimer::timeout,this,&ArgusCamera::initCAM);
//    this->exec();

}



ArgusSamples::Value<uint32_t> cameraIndex(DEFAULT_CAMERA_INDEX);

const uint64_t FIVE_SECONDS_IN_NANOSECONDS = 5000000000;
std::vector<Argus::CameraDevice*> cameraDevices;

/*
 * Set up Argus API Framework, identify available camera devices, and create
 * a capture session for the first available device
 */



bool triggerMode::initCAM(){

    Argus::UniqueObj<Argus::CameraProvider> cameraProvider(Argus::CameraProvider::create());
    Argus::ICameraProvider *iCameraProvider = Argus::interface_cast<Argus::ICameraProvider>(cameraProvider);
    EXIT_IF_NULL(iCameraProvider, "Cannot get core camera provider interface");
    printf("Argus Version: %s\n", iCameraProvider->getVersion().c_str());

    Argus::Status status = iCameraProvider->getCameraDevices(&cameraDevices);
    EXIT_IF_NOT_OK(status, "Failed to get camera devices");
    EXIT_IF_NULL(cameraDevices.size(), "No camera devices available");

    if (cameraDevices.size() <= cameraIndex.get())
    {
        printf("Camera device specifed on command line is not available\n");
        return EXIT_FAILURE;
    }

    Argus::UniqueObj<Argus::CaptureSession> captureSession(iCameraProvider->createCaptureSession(cameraDevices[cameraIndex.get()], &status));
    Argus::ICaptureSession *iSession = Argus::interface_cast<Argus::ICaptureSession>(captureSession);
    EXIT_IF_NULL(iSession, "Cannot get Capture Session Interface");

    /*
     * Creates the stream between the Argus camera image capturing
     * sub-system (producer) and the image acquisition code (consumer).  A consumer object is
     * created from the stream to be used to request the image frame.  A successfully submitted
     * capture request activates the stream's functionality to eventually make a frame available
     * for acquisition.
     */


    Argus::UniqueObj<Argus::OutputStreamSettings> streamSettings(iSession->createOutputStreamSettings());

    Argus::IOutputStreamSettings *iStreamSettings = Argus::interface_cast<Argus::IOutputStreamSettings>(streamSettings);
    EXIT_IF_NULL(iStreamSettings, "Cannot get OutputStreamSettings Interface");
    iStreamSettings->setPixelFormat(Argus::PIXEL_FMT_YCbCr_420_888);
    iStreamSettings->setResolution(Argus::Size2D<uint32_t>(640, 480));
    iStreamSettings->setMetadataEnable(true);

    Argus::UniqueObj<Argus::OutputStream> stream(iSession->createOutputStream(streamSettings.get()));
    Argus::IStream *iStream = Argus::interface_cast<Argus::IStream>(stream);
    EXIT_IF_NULL(iStream, "Cannot get OutputStream Interface");

    Argus::UniqueObj<EGLStream::FrameConsumer> consumer(EGLStream::FrameConsumer::create(stream.get()));

    EGLStream::IFrameConsumer *iFrameConsumer = Argus::interface_cast<EGLStream::IFrameConsumer>(consumer);
    EXIT_IF_NULL(iFrameConsumer, "Failed to initialize Consumer");

    Argus::UniqueObj<Argus::Request> request(iSession->createRequest(Argus::CAPTURE_INTENT_STILL_CAPTURE));
    Argus::IRequest *iRequest = Argus::interface_cast<Argus::IRequest>(request);
    EXIT_IF_NULL(iRequest, "Failed to get capture request interface");


//    bool connection;

    //Signal Connection( DetectValue ) in Argus Camera, Slot(  ) in Windows)
    //Or Call back Function


    return EXIT_SUCCESS;



}



void triggerMode::prepareStop(bool stopButton)
{
    stopButtonPressed = true;
    cout<< "Stop Button: " << stopButtonPressed << endl;
}


void triggerMode::testfunction(){



}
