#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <string>

const int StartSet = 1;
const int FinishSet = 6001; // not inclusive!
const bool Print = false;

const int nThreads = 150;
const int nSetsPerThread = 20;

//const std::string dataPath = "/home/kate/imat2011/data/result_jpg_300/";
const std::string dataPath = "/home/kate/imat2011/data/result/";

const std::string learningFile = "/home/kate/imat2011/data/learning.txt";
const int nImagesInSet = 5;

const float matchLimit = 0.4;

// for image stitching
double const LastMinSquaredDistancePercent = 0.4;
int const HessianTreshold = 500;
int const CvSurfParamsExtended = 0;

// for template matching
int const FmtMatchPercentage = 75;
int const VerticalDivisionLimit  = 3;

#endif // PARAMETERS_H
