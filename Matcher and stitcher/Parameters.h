#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <string>

const int StartSet = 1;
const int FinishSet = StartSet+100; // not inclusive!
const bool Print = false;

const std::string dataPath = "/home/kate/imat2011/data/result_jpg_300/";
const std::string learningFile = "/home/kate/imat2011/data/learning.txt";
const int nImagesInSet = 5;

const float matchLimit = 0.46;

// for image stitching
double const LastMinSquaredDistancePercent = 0.2;
int const HessianTreshold = 500;
int const CvSurfParamsExtended = 0;

// for template matching
int const FmtMatchPercentage = 75;
int const VerticalDivisionLimit  = 3;

#endif // PARAMETERS_H
