#include "camera.h"
#include <QString>
#include <iostream>
#include <string>
#include <algorithm>
#include <math.h>
#include "utils/qtpreviewconsumer.h"
#include "utils/Error.h"
#include "utils/Options.h"
#include <nvbuf_utils.h>
#include "EGLStream/NV/ImageNativeBuffer.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d.hpp>

#include "sys/types.h"
#include "sys/sysinfo.h"
#include <sys/time.h>
#include <unistd.h> //for sleep
#include <chrono> //for time
#include <sys/mman.h> //for mmap
#include "asmOpenCV.h"

using namespace Argus;
using namespace std;

Camera::Camera(int camDeviceIndex) {

    this->cameraDeviceIndex = camDeviceIndex;

    this->restarted = false;

    this->stopButtonPressed = false;
    this->pauseButtonPressed = false;
    this->captureButtonPressed = false;

    this->captureMode = 0;
    this->gain = 0;
    this->exposure = 0;

    this->frameCaptureCount=0;

    this->previousTimeStamp=0.0;
    this->sensorTimeStamp=0.0;
}

bool Camera::startSession() {

    cout << "cameraDeviceIndex " << this->cameraDeviceIndex << endl;

    if (!restarted) {

        //CAMERA PROVIDER
        this->cameraProvider = UniqueObj<CameraProvider>(CameraProvider::create());
        this->iCameraProvider = interface_cast<ICameraProvider>(this->cameraProvider);
        EXIT_IF_NULL(this->iCameraProvider, "Cannot get core camera provider interface");

        //CAMERA DEVICE
        this->status = this->iCameraProvider->getCameraDevices(&(this->cameraDevices));
        EXIT_IF_NOT_OK(this->status, "Failed to get camera devices");
        EXIT_IF_NULL(this->cameraDevices.size(), "No camera devices available");
        cout << "There are " << this->cameraDevices.size() << " camera ports detected. " <<  endl;
    }

    //CAPTURE SESSION
    this->captureSession = UniqueObj<CaptureSession>(this->iCameraProvider->createCaptureSession(this->cameraDevices[this->cameraDeviceIndex]));
    this->iSession = interface_cast<ICaptureSession>(this->captureSession);
    EXIT_IF_NULL(this->iSession, "Cannot get Capture Session Interface");

    //OUTPUT STREAM SETTINGS
    this->streamSettings = UniqueObj<OutputStreamSettings>(this->iSession->createOutputStreamSettings());
    this->iStreamSettings =interface_cast<IOutputStreamSettings>(this->streamSettings);
    EXIT_IF_NULL(this->iStreamSettings, "Cannot get OutputStreamSettings Interface");
    this->iStreamSettings->setPixelFormat(PIXEL_FMT_YCbCr_420_888);
    this->iStreamSettings->setResolution(Size2D<uint32_t>(1920, 1080));
    this->iStreamSettings->setEGLDisplay(this->g_display.get());

    //CREATE OUTPUT STREAM
    this->stream = UniqueObj<OutputStream>(this->iSession->createOutputStream(this->streamSettings.get()));
    this->iStream = interface_cast<IStream>(this->stream);
    EXIT_IF_NULL(this->iStream, "Cannot get OutputStream Interface");

    Argus::UniqueObj<EGLStream::FrameConsumer> consumer(EGLStream::FrameConsumer::create(this->stream.get()));
    this->iFrameConsumer = Argus::interface_cast<EGLStream::IFrameConsumer>(consumer);
    EXIT_IF_NULL(this->iFrameConsumer, "Failed to initialize Consumer");

    //REQUEST
    this->request = UniqueObj<Request>(this->iSession->createRequest(CAPTURE_INTENT_MANUAL));
    this->iRequest = interface_cast<IRequest>(this->request);
    EXIT_IF_NULL(this->iRequest, "Failed to get capture request interface");

    this->status = this->iRequest->enableOutputStream(this->stream.get());
    EXIT_IF_NOT_OK(this->status, "Failed to enable stream in capture request");

    uint32_t requestId = this->iSession->capture(this->request.get());
    EXIT_IF_NULL(requestId, "Failed to submit capture request");

    //SETTINGS INTERFACE
    this->iSourceSettings = interface_cast<ISourceSettings>(this->iRequest->getSourceSettings());
    EXIT_IF_NULL(this->iSourceSettings, "Failed to get source settings interface");

    //AUTOCONTROL SETTING INTERFACE
    this->iAutoControlSettings = interface_cast<IAutoControlSettings>(this->iRequest->getAutoControlSettings());
    EXIT_IF_NULL(this->iAutoControlSettings, "Failed to get source settings interface");

    //DENOISE SETTING INTERFACE
    this->iDenoiseSettings = interface_cast<IDenoiseSettings>(this->request);
    EXIT_IF_NULL(this->iDenoiseSettings, "Failed to get source settings interface");

    this->iDenoiseSettings->setDenoiseMode(DENOISE_MODE_OFF);
    this->iDenoiseSettings->setDenoiseStrength(0.0f);

    //CAMERA PROPERTIES
    this->iCameraProperties = interface_cast<ICameraProperties>(this->cameraDevices[this->cameraDeviceIndex]);
    EXIT_IF_NULL(this->iCameraProperties, "Failed to get ICameraProperties interface");

    std::vector<SensorMode*> sensorModes;
    this->iCameraProperties->getBasicSensorModes(&sensorModes);
    std::vector<SensorMode*> modes;
    this->iCameraProperties->getAllSensorModes(&modes);
    if (sensorModes.size() == 0)
        cout <<"Failed to get sensor modes"<<endl;

    //SENSOR MODE
    // Index as 2 is 60fps, 0 is 30 fps (fix at 0 for now)
    SensorMode *sensorMode = sensorModes[0];
    this->iSensorMode = interface_cast<ISensorMode>(sensorModes[0]);
    EXIT_IF_NULL(this->iSensorMode, "Failed to get sensor mode interface");

    emit returnRes(this->iSensorMode->getResolution().height(), this->cameraDeviceIndex);

    EXIT_IF_NOT_OK(this->iSourceSettings->setSensorMode(sensorMode),"Unable to set Sensor Mode");

    //INTIALIZES THE CAMERA PARAMETERS OF THE CAMERA STARTING
    EXIT_IF_NOT_OK(this->iRequest->enableOutputStream(stream.get()),"Failed to enable stream in capture request");

    EXIT_IF_NOT_OK(this->iSession->repeat(this->request.get()), "Unable to submit repeat() request");

    this->startTime = std::chrono::high_resolution_clock::now();
    this->finishTime = std::chrono::high_resolution_clock::now();


    //EVENT PROVIDER
    this->iEventProvider = interface_cast<IEventProvider>(this->captureSession);
    EXIT_IF_NULL(this->iEventProvider, "iEventProvider is NULL");

    std::vector<EventType> eventTypes;
    eventTypes.push_back(EVENT_TYPE_CAPTURE_COMPLETE);

    this->queue = UniqueObj<EventQueue>(this->iEventProvider->createEventQueue(eventTypes));
    this->iQueue = interface_cast<IEventQueue>(this->queue);
    EXIT_IF_NULL(this->iQueue, "event queue interface is NULL");

    cout << "About to start running" << endl;
    runCts();
}

void Camera::endSession() {

    cout << "Ending Capture Session " << this->cameraDeviceIndex << endl;

    this->iRequest->disableOutputStream(this->stream.get());

    this->iSession->stopRepeat();
    this->iSession->waitForIdle();

    this->iStream->disconnect();

    this->stream.reset();
    this->stream.release();

    this->cameraProvider.reset();
    this->cameraProvider.release();

    this->g_display.cleanup();
    cout << "Cleaning Up Display" << this->cameraDeviceIndex << endl;

    emit finished();
}

bool Camera::restartSession() {

    this->restarted = true;

    this->stopButtonPressed = false;
    this->pauseButtonPressed = false;
    this->captureButtonPressed = false;

    this->captureMode = 0;
    this->gain = 0;
    this->exposure = 0;

    this->frameCaptureCount=0;

    this->previousTimeStamp=0.0;
    this->sensorTimeStamp=0.0;

    this->captureSession.reset();
    this->captureSession.release();

    cout << "About to restart Capture" << endl;

    this->startSession();
}

bool Camera::runCts() {
    while (!stopButtonPressed) {

        while(pauseButtonPressed){
            sleep(1);
        }
        emit requestFrameSettings(this->cameraDeviceIndex);

        ///WAIT FOR EVENTS TO GET QUEUED
        this->iEventProvider->waitForEvents(this->queue.get(), 2*ONE_SECOND);
        EXIT_IF_TRUE(this->iQueue->getSize() == 0, "No events in queue");

        ///GET EVENT CAPTURE
        const Event* event = this->iQueue->getEvent(this->iQueue->getSize() - 1);
        const IEventCaptureComplete *iEventCaptureComplete = interface_cast<const IEventCaptureComplete>(event);
        EXIT_IF_NULL(iEventCaptureComplete, "Failed to get EventCaptureComplete Interface");

        ///GET METADATA
        const CaptureMetadata *metaData = iEventCaptureComplete->getMetadata();
        this->iMetadata = interface_cast<const ICaptureMetadata>(metaData);
        EXIT_IF_NULL(iMetadata, "Failed to get CaptureMetadata Interface");

        ///SUPPORTED FRAME RATE
        this->previousTimeStamp=this->sensorTimeStamp;
        this->sensorTimeStamp = this->iMetadata->getSensorTimestamp();
        //printf("Frame Rate (Processing Time) %f\n", 1.0/(SensorTimestamp/1000000000.0-PreviousTimeStamp/1000000000.0));

        /// SET EXPOSURE TIME WITH UI
        EXIT_IF_NOT_OK(this->iSourceSettings->setExposureTimeRange(ArgusSamples::Range<uint64_t>(this->exposure)),"Unable to set the Source Settings Exposure Time Range");

        ///SET GAIN WITH UI
        EXIT_IF_NOT_OK(this->iSourceSettings->setGainRange(ArgusSamples::Range<float>(this->gain)), "Unable to set the Source Settings Gain Range");

        ///FIX ISP GAIN MANUALLY
        EXIT_IF_NOT_OK(this->iAutoControlSettings->setIspDigitalGainRange(ArgusSamples::Range<float>(1.0)), "Unable to Set ISP Gain Value");

        frameRequest();
    }
}

bool Camera::frameRequest() {

    cout << endl << "Camera " << this->cameraDeviceIndex << " Frame: " << this->frameCaptureCount << endl;

    //    if (this->captureMode == 1) {
    //        ///WAIT FOR EVENTS TO GET QUEUED
    //        this->iEventProvider->waitForEvents(this->queue.get(), 2*ONE_SECOND);
    //        EXIT_IF_TRUE(this->iQueue->getSize() == 0, "No events in queue");

    //        ///GET EVENT CAPTURE
    //        const Event* event = this->iQueue->getEvent(this->iQueue->getSize() - 1);
    //        const IEventCaptureComplete *iEventCaptureComplete = interface_cast<const IEventCaptureComplete>(event);
    //        EXIT_IF_NULL(iEventCaptureComplete, "Failed to get EventCaptureComplete Interface");

    //        ///GET METADATA
    //        const CaptureMetadata *metaData = iEventCaptureComplete->getMetadata();
    //        this->iMetadata = interface_cast<const ICaptureMetadata>(metaData);
    //        EXIT_IF_NULL(iMetadata, "Failed to get CaptureMetadata Interface");

    //        ///SUPPORTED FRAME RATE
    //        this->previousTimeStamp=this->sensorTimeStamp;
    //        this->sensorTimeStamp = this->iMetadata->getSensorTimestamp();
    //        //printf("Frame Rate (Processing Time) %f\n", 1.0/(SensorTimestamp/1000000000.0-PreviousTimeStamp/1000000000.0));

    //        /// SET EXPOSURE TIME WITH UI
    //        EXIT_IF_NOT_OK(this->iSourceSettings->setExposureTimeRange(ArgusSamples::Range<uint64_t>(this->exposure)),"Unable to set the Source Settings Exposure Time Range");

    //        ///SET GAIN WITH UI
    //        EXIT_IF_NOT_OK(this->iSourceSettings->setGainRange(ArgusSamples::Range<float>(this->gain)), "Unable to set the Source Settings Gain Range");

    //        ///FIX ISP GAIN MANUALLY
    //        EXIT_IF_NOT_OK(this->iAutoControlSettings->setIspDigitalGainRange(ArgusSamples::Range<float>(1.0)), "Unable to Set ISP Gain Value");
    //    }

    /// START IMAGE GENERATION

    EXIT_IF_NULL(this->iFrameConsumer, "Frame Consumer Invalid")
            Argus::UniqueObj<EGLStream::Frame> frame(this->iFrameConsumer->acquireFrame());
    EGLStream::IFrame *iFrame = Argus::interface_cast<EGLStream::IFrame>(frame);
    EXIT_IF_NULL(iFrame, "Failed to get IFrame interface");

    EGLStream::Image *image = iFrame->getImage();
    EXIT_IF_NULL(image, "Failed to get Image from iFrame->getImage()");

    // Cast image to an IImageNativeBuffer
    EGLStream::NV::IImageNativeBuffer *iImageNativeBuffer = interface_cast<EGLStream::NV::IImageNativeBuffer> (image);
    EXIT_IF_NULL(iImageNativeBuffer, "Failed to get iImageNativeBuffer");
    Argus::Size2D<uint32_t> size(1920, 1080); //1920, 1080//1280,720

    // (Direct Memory Access Buffer File Directory) (YUV420 is similar to Y'CbCr. YUV is analog and Y'CbCr is digital)
    int dmabuf_fd = iImageNativeBuffer->createNvBuffer(size, NvBufferColorFormat_YUV420,NvBufferLayout_Pitch);

    std::vector<cv::Mat> channels;
    std::vector<cv::Mat> resizedChannels;
    cv::Mat img;

    // Pointers to three dma address planes
    void *data_mem1;
    void *data_mem2;
    void *data_mem3;

    channels.clear();

    /// START MAPPING (essentially optimizing memory access for the CPU)

    NvBufferMemMap(dmabuf_fd, 0, NvBufferMem_Read_Write, &data_mem1);
    NvBufferMemSyncForCpu(dmabuf_fd, 0 , &data_mem1);
    //NvBufferMemSyncForDevice(dmabuf_fd, 0 , &data_mem1);
    // CV_8UC1 means a 8-bit single-channel array
    channels.push_back(cv::Mat(1080, 1920, CV_8UC1, data_mem1, 2048));//540, 960 // 1080, 1920 // 720, 1280 //480 , 640 //360,480

    NvBufferMemMap(dmabuf_fd, 1, NvBufferMem_Read_Write, &data_mem2);
    NvBufferMemSyncForCpu(dmabuf_fd, 1 , &data_mem2);
    //NvBufferMemSyncForDevice(dmabuf_fd, 1 , &data_mem2);
    channels.push_back(cv::Mat(540, 960, CV_8UC1, data_mem2,1024)); //270, 480//540, 960 //360,640 //180,240

    NvBufferMemMap(dmabuf_fd, 2, NvBufferMem_Read_Write, &data_mem3);
    NvBufferMemSyncForCpu(dmabuf_fd, 2 , &data_mem3);
    //NvBufferMemSyncForDevice(dmabuf_fd, 2 , &data_mem3);
    channels.push_back(cv::Mat(540, 960, CV_8UC1, data_mem3, 1024)); //23040

    cv::Mat J,K,L;

    resize(channels[0], J,cv::Size(960, 540), 0, 0, cv::INTER_AREA); //640, 480 //960, 540 //480, 270
    //resize(channels[1], K,cv::Size(640, 480), 0, 0, cv::INTER_AREA);
    //resize(channels[2], L,cv::Size(640, 480), 0, 0, cv::INTER_AREA);

    K=channels[1];
    L=channels[2];

    resizedChannels.push_back(J);
    resizedChannels.push_back(K);
    resizedChannels.push_back(L);

    // merge() merges several arrays to make a single, multi-channel array (img)
    // Think a 2-D array where each element in the array has multiple valies (RGB, YUV etc). This is a multi-channel array
    // A single channel array would just have one value per element
    cv::merge ( resizedChannels, img );
    // cvtColor() converts from the YCrCb space to RGB color space
    cv::cvtColor ( img,img,CV_YCrCb2RGB );

    /// START IMAGE PROCESSING
    /// The goal of this processing pipeline is to have the image in a floodfill binary space so that defects can be identified
    ///

    Mat imgProc, imgTh, imgGray, imgFF;

    imgProc=img.clone();

    // cvtColor() converts from the RGB (or BGR) space to grayscale-channeled image (multichannel to singe channel so we can apply thresholding)
    cv::cvtColor( imgProc, imgGray, CV_BGR2GRAY );
    // threshold() is used here to get a bi-level (binary) image out of a grayscale one with a max value of 255
    // 255 will indicate an abnormality; a defect
    cv::threshold(imgGray,imgTh,150,255,THRESH_BINARY_INV);

    // (960 by 540)
    imgFF=imgTh.clone();

    // floodfill() starts at a seed point and fills the component with the specified color
    cv::floodFill(imgFF,cv::Point(5,5),Scalar(0));

    // ccomp is used to compose the bouding are for the "red circle" around the defect
    Rect ccomp;
    Rect roi(330,0,320,540);


    for(int m = roi.y; m < (roi.y + roi.height); m++) {

        for(int n = roi.x; n < (roi.x+roi.width); n++) {
            int iPixel=imgFF.at<uchar>(m,n);

            if(iPixel==255) {

                int iArea=floodFill(imgFF,Point(n,m),Scalar(50),&ccomp);
                if(iArea<75) {
                    floodFill(imgFF,Point(n,m),Scalar(0));
                }
                else {
                    circle(imgProc,Point(ccomp.x+ccomp.width/2,ccomp.y+ccomp.height/2),40,Scalar(0,0,255),2,LINE_8);
                    this->defectFound = true;
                }
            }
        }
    }

    if (this->captureButtonPressed) {
        string savepath = "/home/nvidia/capture" + std::to_string(this->cameraDeviceIndex) + "_" + std::to_string(this->frameCaptureCount) + ".png";
        cv::imwrite(savepath, (imgProc)(roi));
        this->captureButtonPressed = false;
    }

    // Display different processed images by setting DisplayIndex (default to 1)
    imShow[this->cameraDeviceIndex][1]=imgProc.clone();
    imShow[this->cameraDeviceIndex][2]=imgTh.clone();
    imShow[this->cameraDeviceIndex][3]=imgFF.clone();
    imShow[this->cameraDeviceIndex][4]=imgGray.clone();

    //QImage  QImg((uchar*) img.data, img.cols, img.rows, img.step, QImage::Format_RGB888 );
    Mat tej = imShow[this->cameraDeviceIndex][this->DisplayIndex](roi);
    QImage QImgDefect = ASM::cvMatToQImage(tej);

    //emit returnQImage(Qimg.rgbSwapped(), this->cameraDeviceIndex);
    emit returnQDefectImage(QImgDefect, this->cameraDeviceIndex);

    if (this->defectFound) {
        emit returnQPrevDefectImage(QImgDefect, this->cameraDeviceIndex);
        this->defectFound = false;
    }


    //START UNMAPPING
    if ((this->frameCaptureCount)%10 == 0) {
        this->iSession->repeat(this->request.get());
    }

    NvBufferMemUnMap (dmabuf_fd, 0, &data_mem1);
    NvBufferMemUnMap (dmabuf_fd, 1, &data_mem2);
    NvBufferMemUnMap (dmabuf_fd, 2, &data_mem3);
    NvBufferDestroy (dmabuf_fd);

    //mutex->lock();
    //(*frameFinished)++;
    //cout << "frameFinished:" << *frameFinished << endl;
    this->frameCaptureCount++;

    // Requests a new frame when all three frames have been displayed
    //if(*frameFinished == 3)
    //{
    //  *frameFinished = 0;
    // emit returnFrameFinished(true);
    //}
    //mutex->unlock();

    //if(this->captureMode == 0) {
    this->sensorTimeStamp = this->iMetadata->getSensorTimestamp();

    // Frame Rate Information
    finishTime = std::chrono::high_resolution_clock::now();
    // In nanosecond ticks
    float totalDuration= std::chrono::duration_cast<std::chrono::nanoseconds>(finishTime-startTime).count();

    emit returnFrameRate((this->frameCaptureCount*1.0)/(totalDuration/1000000000.0), this->cameraDeviceIndex);
    emit returnCurrFrameRate(1.0/(this->sensorTimeStamp/1000000000.0-previousTimeStamp/1000000000.0), this->cameraDeviceIndex);
    //}

    return true;
}

