// SimpleCV.cpp : Defines the entry point for the console application.


//#include "stdafx.h"
#include "SimpleCV.h"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "mainwindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The one and only application object
//CWinApp theApp;



using namespace std;
using namespace cv;

static bool m_refColour = false;
static double m_LabRef[3] = { 100, -5, -4 };
static int m_colourTolerance = 12;

//int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
//{
//	AnalysisCV(cvLoadImage("Sample Image.jpg", CV_LOAD_IMAGE_COLOR));

//	system("pause");

//	return 0;
//}









std::vector<double> AnalysisCV(IplImage* imgSrc)
{
    int imageWidth = imgSrc->width; int imageHeight = imgSrc->height;
    CvSize srcImageSize = cvGetSize(imgSrc);
    IplImage* imgTemp = cvCreateImage(srcImageSize, 8, 3);
    IplImage* imgCableBody = cvCreateImage(srcImageSize, 8, 1);
    IplImage* imgIsolatedCableBody = cvCreateImage(srcImageSize, 8, 3);
    IplImage* imgSobel = cvCreateImage(srcImageSize, 8, 3);
    IplImage* imgDilate = cvCreateImage(srcImageSize, 8, 3);
    IplImage* imgBlue = cvCreateImage(srcImageSize, 8, 3);
    IplImage* imgGreen = cvCreateImage(srcImageSize, 8, 3);
    IplImage* imgRed = cvCreateImage(srcImageSize, 8, 3);
    IplImage* imgGrayscale = cvCreateImage(srcImageSize, 8, 3);
    IplImage* imgThreshold = cvCreateImage(srcImageSize, 8, 3);

    time_t begin = clock();

    //finds the cable body in the image, threshold value hard set to 150, could be variable
    cvSet(imgIsolatedCableBody, cvScalarAll(0));
    cvSplit(imgSrc, imgCableBody, NULL, NULL, NULL);
    cvThreshold(imgCableBody, imgCableBody, 150, 255, THRESH_BINARY);

    //thresholded image is used as mask to source image to isolated cable body image
    cvCopy(imgSrc, imgIsolatedCableBody, imgCableBody);

    std::vector <double> temp;
    temp.clear();


    if (int(cvAvg(imgIsolatedCableBody).val[0]) < 1){
//        LAB[0]=0;
//        LAB[1]=0;
//        LAB[2]=0;

        temp.push_back(0);
        temp.push_back(0);
        temp.push_back(0);


        cvReleaseImage(&imgTemp);
        cvReleaseImage(&imgCableBody);
        cvReleaseImage(&imgIsolatedCableBody);
        cvReleaseImage(&imgSobel);
        cvReleaseImage(&imgDilate);
        cvReleaseImage(&imgBlue);
        cvReleaseImage(&imgGreen);
        cvReleaseImage(&imgRed);
        cvReleaseImage(&imgGrayscale);
        cvReleaseImage(&imgThreshold);


        delete imgTemp;
        delete imgCableBody;
        delete imgIsolatedCableBody;
        delete imgSobel;
        delete imgDilate;
        delete imgBlue;
        delete imgGreen;
        delete imgRed;
        delete imgGrayscale;
        delete imgThreshold;



        return temp;
     }

    Mat sourceImage = cvarrToMat(imgIsolatedCableBody);
    Mat samples = sourceImage.reshape(1, sourceImage.rows * sourceImage.cols);

    // sorts the image to put the background on top of the image and the cable colours at bottom
    time_t begin2 = clock();
    cv::sort(samples, samples, CV_SORT_EVERY_COLUMN);
    time_t end2 = clock();
    double timepass2 = difftime(end2, begin2) / CLOCKS_PER_SEC;

    cout << "Sort Time: " << timepass2 << endl;

    int halfImagePosition = floor(imageWidth / 2);
    double LAB[3];
    double mBGR[3];


    // goes through sorted image to find where the background starts
    for (int iHeight = imageHeight - 1; iHeight > 0; iHeight--)
    {
        // get the pixel value at current height
        int pixel = ((uchar*)(imgIsolatedCableBody->imageData + imgIsolatedCableBody->widthStep*iHeight))[halfImagePosition];

        // find first black pixel
        if (pixel == 0 && iHeight < imageHeight - 4)
        {
            // isolate colour area
            cvSetImageROI(imgIsolatedCableBody, cvRect(0, iHeight + floor((imageHeight - iHeight) / 4), imageWidth, imageHeight - floor((imageHeight - iHeight) / 4)));

            // find average colour of isolated area
            CvScalar scMeasuredColour = cvAvg(imgIsolatedCableBody);

            double XYZ[3];
            double BGR[3] = { scMeasuredColour.val[0], scMeasuredColour.val[1], scMeasuredColour.val[2] };

            mBGR[0] = scMeasuredColour.val[0];
            mBGR[1] = scMeasuredColour.val[1];
            mBGR[2] = scMeasuredColour.val[2];


            //cout << "B  " << BGR[0] << "  G   " << BGR[1] << "  R   " << BGR[2] <<endl;
            // convert average colour to Lab colour space
            CvtBGRtoXYZ(BGR, XYZ);
            CvtXYZtoLAB(XYZ, LAB);

            break;
        }
    }

    // if reference colour flag set true, set measured colours as reference colours
    if (m_refColour)
    {
        m_LabRef[0] = LAB[0];
        m_LabRef[1] = LAB[1];
        m_LabRef[2] = LAB[2];

        m_refColour = false;
    }
    else
    {
        // find colour difference between measured and reference
        double deltaE = ColourDifference(LAB[0], LAB[1], LAB[2], m_LabRef[0], m_LabRef[1], m_LabRef[2]);

        time_t end = clock();

        double timepass2 = difftime(end, begin) / CLOCKS_PER_SEC;

        cout << "Time taken to execute: " << timepass2 << endl;

        // colour tolerance point hard coded at top
        if (deltaE > m_colourTolerance)
            cout << "Fail: " << deltaE << endl << "Measured Colour: L " << LAB[0] << " a " << LAB[1] << " b " << LAB[2] << endl;
        else
            cout << "Pass: " << deltaE << endl << "Measured Colour: L " << LAB[0] << " a " << LAB[1] << " b " << LAB[2] << endl;
    }


    cvReleaseImage(&imgTemp);
    cvReleaseImage(&imgCableBody);
    cvReleaseImage(&imgIsolatedCableBody);
    cvReleaseImage(&imgSobel);
    cvReleaseImage(&imgDilate);
    cvReleaseImage(&imgBlue);
    cvReleaseImage(&imgGreen);
    cvReleaseImage(&imgRed);
    cvReleaseImage(&imgGrayscale);
    cvReleaseImage(&imgThreshold);


    delete imgTemp;
    delete imgCableBody;
    delete imgIsolatedCableBody;
    delete imgSobel;
    delete imgDilate;
    delete imgBlue;
    delete imgGreen;
    delete imgRed;
    delete imgGrayscale;
    delete imgThreshold;


    temp.push_back(LAB[0]);
    temp.push_back(LAB[1]);
    temp.push_back(LAB[2]);

    temp.push_back(mBGR[0]);
    temp.push_back(mBGR[1]);
    temp.push_back(mBGR[2]);


    return temp;
}


void CvtBGRtoXYZ(double BGR[], double XYZ[])
{
    double var_R = BGR[2] / 255.0;        //R from 0 to 255
    double var_G = BGR[1] / 255.0;        //G from 0 to 255
    double var_B = BGR[0] / 255.0;        //B from 0 to 255

    if (var_R > 0.04045)
        var_R = pow((var_R + 0.055) / 1.055, 2.4);
    else
        var_R = var_R / 12.92;
    if (var_G > 0.04045)
        var_G = pow((var_G + 0.055) / 1.055, 2.4);
    else
        var_G = var_G / 12.92;
    if (var_B > 0.04045)
        var_B = pow((var_B + 0.055) / 1.055, 2.4);
    else
        var_B = var_B / 12.92;

    var_R = var_R * 100.0;
    var_G = var_G * 100.0;
    var_B = var_B * 100.0;

    //Observer. = 2�, Illuminant = D65
    XYZ[0] = var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805;
    XYZ[1] = var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722;
    XYZ[2] = var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505;
}

void CvtXYZtoLAB(double XYZ[], double Lab[])
{
    double ref_X, ref_Y, ref_Z, var_X, var_Y, var_Z = 0;

    //Observer= 2�, Illuminant= D65
    //ref_X =  95.047 ;
    //ref_Y = 100.000;
    //ref_Z = 108.883;

    //Observer= 10�, Illuminant= D65
    ref_X = 94.811;
    ref_Y = 100.00;
    ref_Z = 107.304;

    var_X = XYZ[0] / ref_X;
    var_Y = XYZ[1] / ref_Y;
    var_Z = XYZ[2] / ref_Z;

    if (var_X > 0.008856)
        var_X = pow(var_X, 1.0 / 3.0);
    else
        var_X = (841.0 / 108.0 * var_X) + (16.0 / 116.0);

    if (var_Y > 0.008856)
        var_Y = pow(var_Y, 1.0 / 3.0);
    else
        var_Y = (841.0 / 108.0 * var_Y) + (16.0 / 116.0);

    if (var_Z > 0.008856)
        var_Z = pow(var_Z, 1.0 / 3.0);
    else
        var_Z = (841.0 / 108.0 * var_Z) + (16.0 / 116.0);

    Lab[0] = 116.0 * var_Y - 16.0;
    Lab[1] = 500.0 * (var_X - var_Y);
    Lab[2] = 200.0 * (var_Y - var_Z);
}

double ColourDifference(double L_t, double a_t, double b_t, double L_r, double a_r, double b_r)
{
    double C_t, a_p_t, C_p_t, h_p_t, C_r, a_p_r, C_p_r, L_p_avg, C_avg, C_p_avg, h_p_r, h_p_avg, G, T, delta_h_p, delta_L_p, delta_C_p, delta_H_p, S_L, S_C, S_H, delta_theta, R_C, R_T, K_L, K_C, K_H, delta_E = 0;

    //L_t = 23.0331;
    //a_t = 14.9730;
    //b_t = -42.5619;

    //L_r = 22.7233;
    //a_r = 20.0904;
    //b_r = -46.6940;

    K_L = 1;
    K_H = 1;
    K_C = 1;

    C_t = sqrt(pow(a_t, 2) + pow(b_t, 2));
    C_r = sqrt(pow(a_r, 2) + pow(b_r, 2));
    L_p_avg = (L_t + L_r) / 2;
    C_avg = (C_t + C_r) / 2;
    G = 0.5*(1 - sqrt(pow(C_avg, 7) / (pow(C_avg, 7) + pow(25.0, 7))));
    a_p_t = a_t * (1 + G);
    a_p_r = a_r * (1 + G);
    C_p_t = sqrt(pow(a_p_t, 2) + pow(b_t, 2));
    C_p_r = sqrt(pow(a_p_r, 2) + pow(b_r, 2));
    C_p_avg = (C_p_t + C_p_r) / 2;
    h_p_t = HueAngle(b_t, a_p_t);
    h_p_r = HueAngle(b_r, a_p_r);
    h_p_avg = (h_p_t + h_p_r) / 2;
    T = 1 - 0.17 * cos((h_p_avg - 30) * PI / 180) + 0.24 * cos((2 * h_p_avg) * PI / 180) + 0.32 * cos((3 * h_p_avg + 6) * PI / 180) - 0.2 * cos((4 * h_p_avg - 63) * PI / 180);
    delta_h_p = h_p_t - h_p_r;
    delta_L_p = L_t - L_r;
    delta_C_p = C_p_t - C_p_r;
    delta_H_p = 2 * sqrt(C_p_t * C_p_r) * sin((delta_h_p / 2) * PI / 180);
    S_L = 1 + (0.015 * pow(L_p_avg - 50, 2)) / sqrt(20 + pow(L_p_avg - 50, 2));
    S_C = 1 + 0.045 * C_p_avg;
    S_H = 1 + 0.015 * C_p_avg * T;
    delta_theta = 30 * exp(-1 * pow((h_p_avg - 275) / 25, 2));
    R_C = 2 * sqrt(pow(C_p_avg, 7) / (pow(C_p_avg, 7) + pow(25.0, 7)));
    R_T = -1 * sin((2 * delta_theta) * PI / 180) * R_C;
    delta_E = sqrt(pow(delta_L_p / K_L / S_L, 2) + pow(delta_C_p / K_C / S_C, 2) + pow(delta_H_p / K_H / S_H, 2) + R_T * (delta_C_p / K_C / S_C) * (delta_H_p / K_H / S_H));

    return delta_E;
}

double HueAngle(double y, double x)
{
    double angle;
    angle = atan2(y, x) * 180 / PI;
    if (angle < 0)
        angle = angle + 360;

    return angle;
}


