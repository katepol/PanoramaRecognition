#include "Stitcher.h"

void Stitcher::printStitchMatrix (std::ostream& out, StitchMatrix const & m)
{
    for (int i=0; i!=nImagesInSet; ++i)
    {
        for (int j=0; j!=nImagesInSet; ++j)
            //out << m(i, j).nMatchPoints << " ";
            out << "(" << m(i, j).nMatchPoints << "; " << m(i, j).avgDistance << "; " << m(i, j).matchPointsToTotalPoints << ") ";
        out << "\n";
    }
}

Stitcher::Stitcher()
{

}

int Stitcher::findNaiveNearestNeighbor(double * distance, const float* image1Descriptor, const CvSURFPoint* image1KeyPoint, CvSeq* image2Descriptors, CvSeq* image2KeyPoints)
{
    int descriptorsCount = (int)(image2Descriptors->elem_size/sizeof(float));
    double minSquaredDistance = std::numeric_limits<double>::max();
    double lastMinSquaredDistance = std::numeric_limits<double>::max();

    int neighbor;
    for (int i = 0; i < image2Descriptors->total; i++) {
        const CvSURFPoint* image2KeyPoint = (const CvSURFPoint*) cvGetSeqElem(image2KeyPoints, i);
        const float* image2Descriptor = (const float*) cvGetSeqElem(image2Descriptors, i);

        if (image1KeyPoint->laplacian != image2KeyPoint->laplacian)
            continue; // Don't worry about key points unless laplacian signs are equal

        double squaredDistance = this->compareSURFDescriptors(image1Descriptor, image2Descriptor, descriptorsCount, lastMinSquaredDistance);

        if (squaredDistance < minSquaredDistance) {
            neighbor = i;
            lastMinSquaredDistance = minSquaredDistance;
            minSquaredDistance = squaredDistance;
        } else if (squaredDistance < lastMinSquaredDistance) {
            lastMinSquaredDistance = squaredDistance;
        }
    }

    if (minSquaredDistance < LastMinSquaredDistancePercent * lastMinSquaredDistance) {
        *distance = minSquaredDistance;
        return neighbor;
    }
    return NO_NEIGHBOR;

}

double Stitcher::compareSURFDescriptors(const float* image1Descriptor, const float* image2Descriptor, int descriptorsCount, float lastMinSquaredDistance)
{
    double totalCost = 0;

    for (int i = 0; i < descriptorsCount; i += 4) {
        QVector4D descriptor1(image1Descriptor[i+0], image1Descriptor[i+1], image1Descriptor[i+2], image1Descriptor[i+3]);
        QVector4D descriptor2(image2Descriptor[i+0], image2Descriptor[i+1], image2Descriptor[i+2], image2Descriptor[i+3]);
        totalCost += (descriptor2 - descriptor1).lengthSquared();
        if (totalCost > lastMinSquaredDistance)
            break;
    }

   return totalCost;
}

QVector<QVector<CvPoint2D32f> > Stitcher::findMatchingKeyPoints(double * avgDistance, CvSeq* image1KeyPoints, CvSeq* image1Descriptors, CvSeq* image2KeyPoints, CvSeq* image2Descriptors)
{
    // Find matching keypoints in both images
    QVector<QVector<CvPoint2D32f> > keyPointMatches;
    keyPointMatches.append(QVector<CvPoint2D32f>());
    keyPointMatches.append(QVector<CvPoint2D32f>());
    double n = 0;
    double sum = 0;
    double d = 500.0;
    for (int i = 0; i < image1Descriptors->total; i++) {
        const CvSURFPoint* image1KeyPoint = (const CvSURFPoint*) cvGetSeqElem(image1KeyPoints, i);
        const float* image1Descriptor =  (const float*) cvGetSeqElem(image1Descriptors, i);
        int nearestNeighbor =
                this->findNaiveNearestNeighbor(&d,
                        image1Descriptor,
                        image1KeyPoint,
                        image2Descriptors,
                        image2KeyPoints
                        );

        if (nearestNeighbor == NO_NEIGHBOR) {
            continue;
        }

        n += 1;
        sum += d;

        keyPointMatches[0].append(((CvSURFPoint*) cvGetSeqElem(image1KeyPoints, i))->pt);
        keyPointMatches[1].append(((CvSURFPoint*) cvGetSeqElem(image2KeyPoints, nearestNeighbor))->pt);
    }
    //logger << "; N matching points = "<< n << "; Average neighbour distance = " << sum/n;
    *avgDistance = sum/n;

    return keyPointMatches;
}

StitchData * Stitcher::findStitchingPoints(QImage &image1, QImage &image2)
{
    IplImage* ipl1 = Stitcher::QImage2GrayscaleIplImage(image1);
    IplImage* ipl2 = Stitcher::QImage2GrayscaleIplImage(image2);

    if (ipl1 == 0 || ipl2 == 0) {
        cvReleaseImage(&ipl1);
        cvReleaseImage(&ipl2);
        return 0;
    }

    CvMemStorage* memoryBlock = cvCreateMemStorage();

    CvSeq* image1KeyPoints;
    CvSeq* image1Descriptors;
    CvSeq* image2KeyPoints;
    CvSeq* image2Descriptors;

    // Only values with a hessian greater than HessianTreshold are considered for keypoints
    CvSURFParams params = cvSURFParams(HessianTreshold, CvSurfParamsExtended);

    try {
        cvExtractSURF(ipl1, 0, &image1KeyPoints, &image1Descriptors, memoryBlock, params);
        cvExtractSURF(ipl2, 0, &image2KeyPoints, &image2Descriptors, memoryBlock, params);
    }
    catch (...) {
        cvReleaseImage(&ipl1);
        cvReleaseImage(&ipl2);
        cvReleaseMemStorage(&memoryBlock);
        return 0;
    }

    //qDebug() << "extracted features: " << image1KeyPoints->total << "; " << image2KeyPoints->total << "; " << image1Descriptors->total<< "; "<<image2Descriptors->total<<"; "<<params.nOctaves << "; "<<params.nOctaveLayers;

    cvReleaseImage(&ipl1);
    cvReleaseImage(&ipl2);

    double avgDistance;
    QVector<QVector<CvPoint2D32f> > matchingKeyPoints = this->findMatchingKeyPoints(&avgDistance,
            image1KeyPoints,
            image1Descriptors,
            image2KeyPoints,
            image2Descriptors);

    int mp = matchingKeyPoints.first().count();
    StitchData * sd = new StitchData(mp, avgDistance, (double)mp/image1KeyPoints->total);
    cvReleaseMemStorage(&memoryBlock);
    return sd;

    /*CvMat image1Points = cvMat(1, matchingKeyPoints.first().count(), CV_32FC2, matchingKeyPoints.first().data());
    CvMat image2Points = cvMat(1, matchingKeyPoints.last().count(), CV_32FC2, matchingKeyPoints.last().data());

    double h[9];
    CvMat H = cvMat(3,3, CV_64F, h);
    try {
        cvFindHomography(&image1Points, &image2Points, &H, CV_RANSAC, 9);
    }
    catch (...) {
        cvReleaseMemStorage(&memoryBlock);
        return false;
    }
    cvReleaseMemStorage(&memoryBlock);
    return true;*/
}


StitchMatrix & Stitcher::calcSetStitch (const int setNumber, const int n, const string & path)
{
    StitchMatrix * result = new StitchMatrix(nImagesInSet, nImagesInSet);
    for (int current=0; current!=n; ++current)
    {
        string imgSource = path + boost::lexical_cast<string>(setNumber) + "_" + boost::lexical_cast<string>(current+1) + ".jpg";
        QImage i1(imgSource.c_str());
        for (int j=current+1; j!=n; ++j)
        {
            if (current == j) continue;
            string imgTarget = path + boost::lexical_cast<string>(setNumber) + "_" + boost::lexical_cast<string>(j+1) + ".jpg";
            QImage i2(imgTarget.c_str());
            StitchData * sd = findStitchingPoints(i1, i2);
            (*result)(current, j) = (sd == 0) ? StitchData() : *sd;
            (*result)(j, current) = (*result)(current, j);
         }
    }
    return *result;
}

Mat_<float> & Stitcher::calcSetStitch2 (const int setNumber, int * nMismatched, const int n, const string & path)
{
    Mat_<float> * result = new Mat_<float>(nImagesInSet, nImagesInSet);
    for (int current=0; current!=n; ++current)
    {
        string imgSource = path + boost::lexical_cast<string>(setNumber) + "_" + boost::lexical_cast<string>(current+1) + ".jpg";
        QImage i1(imgSource.c_str());
        for (int j=current+1; j!=n; ++j)
        {
            string imgTarget = path + boost::lexical_cast<string>(setNumber) + "_" + boost::lexical_cast<string>(j+1) + ".jpg";
            QImage i2(imgTarget.c_str());
            StitchData * sd = findStitchingPoints(i1, i2);
            (*result)(current, j) = (sd == 0) ? 0 : sd->nMatchPoints;
            (*result)(j, current) = (*result)(current, j);
        }
    }

    for (int i=0; i!=n; ++i)
    {
        bool mismatched = true;
        for (int j=0; j!=n; ++j)
        {
            if ((*result)(i, j) != 0)
            {
                mismatched = false;
                break;
            }
        }
        if (mismatched)
            *nMismatched += 1;
    }

    return *result;
}



IplImage* Stitcher::QImage2IplImage(QImage& image)
{
    int channelCount = 3;

    if (image.hasAlphaChannel())
        channelCount = 4;

    image = image.convertToFormat(QImage::Format_RGB888);

    IplImage* iplImage = cvCreateImage(cvSize(image.width(), image.height()), IPL_DEPTH_8U, channelCount);

    uchar* newData = (uchar*)iplImage->imageData;

    memcpy(newData, image.bits(), image.byteCount());

    if (!image.isGrayscale()) {
        cvCvtColor(iplImage, iplImage, CV_RGB2BGR);
    }

    return iplImage;
}

IplImage* Stitcher::QImage2GrayscaleIplImage(QImage& image)
{
    try {
        IplImage* ipl = Stitcher::QImage2IplImage(image);

        if (ipl->nChannels > 1) {
            IplImage* temp = Stitcher::IplImage2Grayscale(ipl);
            cvReleaseImage(&ipl);
            ipl = temp;
        }

        return ipl;
    }
    catch (...) {
        return 0;
    }
}

IplImage* Stitcher::IplImage2Grayscale(IplImage* image)
{
    IplImage* grayscale = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);

    cvCvtColor(image, grayscale, CV_RGB2GRAY);

    return grayscale;
}


