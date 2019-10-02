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

Camera::Camera(QObject *parent) : QThread(parent)
{

}


void Camera::run()
{
    cout << "Starting Camera" << this->cameraDeviceIndex << endl;
    msleep(3000); //Need this
    initCam();
}


void Camera::putFrameInBuffer(Mat &f){
    pos = idx % buffLen; // Cyclic Array
    frameBuffer[pos] = f.clone();
    idx++;
}

// When init returns success or failure, the thread exits through Camera::run()
bool Camera::initCam(){

    cout << "cameraDeviceIndex " << this->cameraDeviceIndex << endl;

    idx = 0;
    stopButtonPressed = false;

    ///////////////////////////////////////////////////////////////
    ///Camera Provider
    ///////////////////////////////////////////////////////////////

    //CAMERA PROVIDER //1
    CameraProvider::create();
    UniqueObj<CameraProvider> cameraProvider(CameraProvider::create()); //global?
    ICameraProvider *iCameraProvider = interface_cast<ICameraProvider>(cameraProvider);
    EXIT_IF_NULL(iCameraProvider, "Cannot get core camera provider interface");
    printf("Argus Version: %s\n", iCameraProvider->getVersion().c_str());


    ///////////////////////////////////////////////////////////////
    ///Camera Device
    ///////////////////////////////////////////////////////////////

    //CAMERA DEVICE //1
    std::vector<CameraDevice*> cameraDevices;
    Argus::Status status = iCameraProvider->getCameraDevices(&cameraDevices);
    EXIT_IF_NOT_OK(status, "Failed to get camera devices");
    EXIT_IF_NULL(cameraDevices.size(), "No camera devices available");
    cout << "There are " << cameraDevices.size() << " camera ports detected. " <<  endl;
    if (cameraDevices.size() <= cameraDeviceIndex)
    {
        printf("Camera device specified on the command line is not available\n");
        return EXIT_FAILURE;
    }


    ///////////////////////////////////////////////////////////////
    ///Capture Sessioin
    ///////////////////////////////////////////////////////////////

    //CAPTURE SESSION
    UniqueObj<CaptureSession> captureSession(iCameraProvider->createCaptureSession(cameraDevices[cameraDeviceIndex]));
    ICaptureSession *iSession = interface_cast<ICaptureSession>(captureSession);
    EXIT_IF_NULL(iSession, "Cannot get Capture Session Interface");


    /////////////////////////////////////////////////////////////////
    ///Output Stream Settings
    /////////////////////////////////////////////////////////////////

    UniqueObj<OutputStreamSettings> streamSettings(iSession->createOutputStreamSettings());
    IOutputStreamSettings *iStreamSettings =interface_cast<IOutputStreamSettings>(streamSettings);
    EXIT_IF_NULL(iStreamSettings, "Cannot get OutputStreamSettings Interface");
    iStreamSettings->setPixelFormat(PIXEL_FMT_YCbCr_420_888);
    iStreamSettings->setResolution(Size2D<uint32_t>(1920, 1080)); //1920, 1080 //640,480
    iStreamSettings->setEGLDisplay(g_display.get());


    /////////////////////////////////////////////////////////////////
    ///Output Stream
    /////////////////////////////////////////////////////////////////

    //CREATE OUTPUT STREAM
    UniqueObj<OutputStream> stream(iSession->createOutputStream(streamSettings.get()));
    IStream *iStream = interface_cast<IStream>(stream);
    EXIT_IF_NULL(iStream, "Cannot get OutputStream Interface");

    ///METHOD 1: Using Frame Consumer to acquire each frame
    Argus::UniqueObj<EGLStream::FrameConsumer> consumer(EGLStream::FrameConsumer::create(stream.get()));
    EGLStream::IFrameConsumer *iFrameConsumer = Argus::interface_cast<EGLStream::IFrameConsumer>(consumer);
    EXIT_IF_NULL(iFrameConsumer, "Failed to initialize Consumer");

    /////////////////////////////////////////////////////////////////
    ///Request
    /////////////////////////////////////////////////////////////////

    Argus::UniqueObj<Argus::Request> request(iSession->createRequest(CAPTURE_INTENT_MANUAL));
    Argus::IRequest *iRequest = Argus::interface_cast<Argus::IRequest>(request);
    EXIT_IF_NULL(iRequest, "Failed to get capture request interface");

    status = iRequest->enableOutputStream(stream.get());
    EXIT_IF_NOT_OK(status, "Failed to enable stream in capture request");

    uint32_t requestId = iSession->capture(request.get());
    EXIT_IF_NULL(requestId, "Failed to submit capture request");


    ///////////////////////////////////////////////////////////////
    ///CAPTURING LOOP
    ///////////////////////////////////////////////////////////////

    uint32_t frameCaptureLoop = 0;

    while(!stopButtonPressed) {

        if (triggerButtonPressed) {

            triggerButtonPressed = false;
            cout << endl << "Camera " << this->cameraDeviceIndex << " Frame: " << frameCaptureLoop << endl;

            /// START IMAGE GENERATION

            Argus::UniqueObj<EGLStream::Frame> frame(iFrameConsumer->acquireFrame());
            EGLStream::IFrame *iFrame = Argus::interface_cast<EGLStream::IFrame>(frame);
            EXIT_IF_NULL(iFrame, "Failed to get IFrame interface");

            EGLStream::Image *image = iFrame->getImage();
            EXIT_IF_NULL(image, "Failed to get Image from iFrame->getImage()");

            EGLStream::NV::IImageNativeBuffer *iImageNativeBuffer = interface_cast<EGLStream::NV::IImageNativeBuffer> (image);
            EXIT_IF_NULL(iImageNativeBuffer, "Failed to get iImageNativeBuffer");
            Argus::Size2D<uint32_t> size(1920, 1080); //1920, 1080//1280,720

            int dmabuf_fd = iImageNativeBuffer->createNvBuffer(size, NvBufferColorFormat_YUV420,NvBufferLayout_Pitch);

            std::vector<cv::Mat> channels;
            std::vector<cv::Mat> RESIZEDchannels;
            cv::Mat img;

            void *data_mem1;
            void *data_mem2;
            void *data_mem3;
            channels.clear();


            /// START MAPPING

            auto startMapping = std::chrono::high_resolution_clock::now();

            NvBufferMemMap(dmabuf_fd, 0, NvBufferMem_Read_Write, &data_mem1);
            NvBufferMemSyncForCpu(dmabuf_fd, 0 , &data_mem1);
            //NvBufferMemSyncForDevice(dmabuf_fd, 0 , &data_mem1);
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

            RESIZEDchannels.push_back(J);
            RESIZEDchannels.push_back(K);
            RESIZEDchannels.push_back(L);

            auto finishMapping = std::chrono::high_resolution_clock::now();

            cv::merge ( RESIZEDchannels, img );
            cv::cvtColor ( img,img,CV_YCrCb2RGB );


            /// START IMAGE PROCESSING

            Mat imgTh;
            Mat imgProc1;
            Mat imgGray;
            Mat imgProc2;
            Mat saveimg2;

            cv::Rect ROI(0, 180, 639, 200);

            imgProc1=img.clone();


            cvtColor( imgProc1, imgGray, CV_BGR2GRAY );
            threshold(imgGray,imgTh,150,255,THRESH_BINARY_INV);

            Mat imgFF=imgTh.clone();
            floodFill(imgFF,cv::Point(5,5),Scalar(0));


            Rect ccomp;

            for(int m=0;m<imgFF.rows;m++)
            {
                for(int n=0;n<imgFF.cols;n++)
                {
                    int iPixel=imgFF.at<uchar>(m,n);
                    if(iPixel==255)
                    {
                        int iArea=floodFill(imgFF,Point(n,m),Scalar(50),&ccomp);
                        if(iArea<40)
                        {
                            floodFill(imgFF,Point(n,m),Scalar(0));
                        }
                        else
                        {
                            circle(imgProc1,Point(ccomp.x+ccomp.width/2,ccomp.y+ccomp.height/2),30,Scalar(0,0,255),2,LINE_8);
                        }
                    }
                }
            }

            auto finishIP = std::chrono::high_resolution_clock::now();

            //string savepath = "/home/nvidia/capture" + std::to_string(this->cameraDeviceIndex) + "_" + std::to_string(frameCaptureLoop) + ".png";
            //cv::imwrite(savepath, imgProc1);

            imShow[this->cameraDeviceIndex][1]=imgProc1.clone();
            imShow[this->cameraDeviceIndex][2]=imgTh.clone();
            imShow[this->cameraDeviceIndex][3]=imgFF.clone();
            imShow[this->cameraDeviceIndex][4]=imgGray.clone();

            QImage  Qimg((uchar*) img.data, img.cols, img.rows, img.step, QImage::Format_RGB888 );
            Mat tej = imShow[cameraDeviceIndex][DisplayIndex]; //cvCopy
            QImage QimgDefect = ASM::cvMatToQImage(tej);

            if (this->cameraDeviceIndex == 0) {
                emit return_QImage1(Qimg.rgbSwapped());
                emit return_DefectImage1(QimgDefect);
            } else if (this->cameraDeviceIndex == 1) {
                emit return_QImage2(Qimg.rgbSwapped());
                emit return_DefectImage2(QimgDefect);
            } else if (this->cameraDeviceIndex == 2) {
                emit return_QImage3(Qimg.rgbSwapped());
                emit return_DefectImage3(QimgDefect);
            }


            /// START UNMAPPING

            if (frameCaptureLoop%10==0){

                iSession->repeat(request.get());
            }

            NvBufferMemUnMap (dmabuf_fd, 0, &data_mem1);
            NvBufferMemUnMap (dmabuf_fd, 1, &data_mem2);
            NvBufferMemUnMap (dmabuf_fd, 2, &data_mem3);
            NvBufferDestroy (dmabuf_fd);

            frameCaptureLoop++;
        }
    }

    iSession->stopRepeat();
    iSession->waitForIdle();
    iStream->disconnect();

    request.reset();
    request.release();

    cameraProvider.reset();
    cameraProvider.release();

    // Exit this thread
    this->exit();
}


void Camera::stopRequest(bool stopButton)
{
    stopButtonPressed = true;
    cout<< "Stop Button: " << stopButtonPressed << endl;
}

void Camera::triggerRequest(bool checked)
{
    triggerButtonPressed = true;
}

