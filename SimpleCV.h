#pragma once

#include "Resource.h"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#define PI 3.141592654

//void AnalysisCV(IplImage* imgSrc);
void CvtBGRtoXYZ(double BGR[], double XYZ[]);
void CvtXYZtoLAB(double XYZ[], double Lab[]);
double ColourDifference(double L_t, double a_t, double b_t, double L_r, double a_r, double b_r);
double HueAngle(double y, double x);
std::vector <double> AnalysisCV(IplImage* imgSrc);

