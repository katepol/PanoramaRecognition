#ifndef MATCHER_H
#define MATCHER_H

#include <iostream>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <cstring>
#include <boost/lexical_cast.hpp>
#include <vector>

#include "FastMatchTemplate.h"
#include "Parameters.h"

using std::vector;
using std::string;
using std::cerr;
using namespace cv;

typedef void (*cmcF)(const CvMat*, const CvMat*, const int, const int, const int, const int, const int, const int, const int, const int, float*);

class Matcher {
private:
    float nPiece4 (const int n);
    CvPoint nextQuadrant (const int currentQ);
    bool getSubMat(const CvMat * src, CvMat * submat, int const x, int const y, int const w, int const h);


    void calcMatchingCoefficient4 (const CvMat* source,
                               const CvMat* target,
                               const int fmtMatchPercentage,
                               const int template_x,
                               const int template_y,
                               const int template_w,
                               const int template_h,
                               const int recursionLimit,
                               const int recursionDepth,
                               const int quadrant,
                               float* answer);


    bool matchWithOthers (Mat_<float>& m, const int fmtMatchPercentage, const int setNumber, const int current, const int n, const string & path);

public:
    Matcher() {}
    // returns matrix of matching coefficients between n images in setNumber set
    Mat_<float> & calcSetMatch (const int setNumber, const int fmtMatchPercentage = FmtMatchPercentage, const int n = nImagesInSet, const string & path = dataPath);
};



#endif // MATCHER_H
