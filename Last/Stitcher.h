#ifndef STITCHER_H
#define STITCHER_H

#include <QImage>
#include <QVector>
#include <QVector4D>
#include <QDebug>
#include <QColor>
#include <QTime>

#include <limits>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <fstream>
#include <QtCore/qglobal.h>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/ublas/matrix.hpp>

#include "StitchData.h"

#include "Parameters.h"

#define NO_NEIGHBOR -1

typedef boost::numeric::ublas::matrix<StitchData> StitchMatrix;

using namespace cv;

class Stitcher {
public:
    Stitcher();


    Mat_<float> & calcSetStitch2 (const int setNumber, int * nMismatched, const int n = nImagesInSet, const string & path = dataPath);

    static void printStitchMatrix (std::ostream& out, StitchMatrix const & m);

private:
    //Stitcher(Stitcher const& other);                    //copy constructor
    Stitcher& operator= (Stitcher const& other);        //operator=

    StitchMatrix & calcSetStitch (const int setNumber, const int n = nImagesInSet, const string & path = dataPath);

    void stitchWithOthers (StitchMatrix& m, const int setNumber, const int current, const int n = nImagesInSet, const string & path = dataPath);
    StitchData * findStitchingPoints(QImage& image1, QImage& image2);

    static IplImage* IplImage2Grayscale(IplImage* image);
    static IplImage* QImage2GrayscaleIplImage(QImage& image);
    static IplImage* QImage2IplImage(QImage& image);

    QVector<QVector<CvPoint2D32f> > findMatchingKeyPoints(double * avgDistance, CvSeq* image1KeyPoints, CvSeq* image1Descriptors, CvSeq* image2KeyPoints, CvSeq* image2Descriptors);
    int findNaiveNearestNeighbor(double * distance, const float* image1Descriptor, const CvSURFPoint* keyPoint, CvSeq* image2Descriptors, CvSeq* image2KeyPoints);
    double compareSURFDescriptors(const float* image1Descriptor, const float* image2Descriptor, int descriptorsCount, float lastMinSquaredDistance);
};

#endif // STITCHER_H
