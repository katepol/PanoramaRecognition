#include "Matcher.h"

float Matcher::nPiece4 (const int n)
{ /* возвращает = какой частью картинки является кусок н-ного уровня деления :)
   * мы делим на 4, потом каждую часть еще на 4 ит.д.*/
    if (n==0)
        return 1;
    return nPiece4(n-1)/4;
}

CvPoint Matcher::nextQuadrant (const int currentQ)
{
    switch (currentQ)
    {
        case 1:
            return cvPoint(1, 0);
        case 2:
            return cvPoint(-1, 1);
        case 3:
            return cvPoint(1, 0);
        default:
            return cvPoint(0, 0);
    }
}

bool Matcher::getSubMat(const CvMat * src, CvMat * submat, int const x, int const y, int const w, int const h)
{
    // test if we are inside the source image
    if (x + 10 >= src->width || w < 10 || y + h/2 >= src->height || x < 0 || y < 0 )
        return false;
    int w_ = std::min(src->width-x, w);
    int h_ = std::min(src->height-y, h);
    cvGetSubRect(src, submat, cvRect(x, y, w_, h_));
    if (submat == 0)
        return false;
    return true;
}

// пилим картинку target на 4 части recursionLimit раз и делаем для них FastMatchTemplate
void Matcher::calcMatchingCoefficient4 (const CvMat* source,
                           const CvMat* target,
                           const int fmtMatchPercentage,
                           const int template_x,
                           const int template_y,
                           const int template_w,
                           const int template_h,
                           const int recursionLimit,
                           const int recursionDepth,
                           const int quadrant,
                           float* answer)
{
    CvMat* submat = new CvMat();
    if (!getSubMat(target, submat, template_x, template_y, template_w, template_h))
    {
        delete submat;
        return;
    }

    // try to find match
    vector<Point> foundPointsList;
    vector<double> confidencesList;
    Mat targetMat(submat);
    Mat sourceMat(source);
    bool foundMatch = FastMatchTemplate(sourceMat, targetMat, &foundPointsList, &confidencesList, fmtMatchPercentage);

    if(!foundMatch && recursionDepth < recursionLimit)
    { // divide into 4 parts and search for matches again
        calcMatchingCoefficient4(source, submat, fmtMatchPercentage, 0, 0, template_w/2, template_h/2, recursionLimit,recursionDepth+1, 1, answer);
    }
    if (foundMatch)
    {
        *answer += confidencesList[0]*nPiece4(recursionDepth);
        delete submat;
        return;
    }
    // try to find matches for the rest parts
    if (quadrant == 4)
    {
        delete submat;
        return;
    }
    CvPoint shift = nextQuadrant(quadrant);
    calcMatchingCoefficient4(source, target, fmtMatchPercentage, template_x+shift.x*template_w, template_y+shift.y*template_h, template_w, template_h, recursionLimit,recursionDepth, quadrant+1, answer);
}



bool Matcher::matchWithOthers (Mat_<float>& m, const int fmtMatchPercentage, const int setNumber, const int current, const int n, const string & path)
{
    string imgSource = path + boost::lexical_cast<string>(setNumber) + "_" + boost::lexical_cast<string>(current+1) + ".jpg";
    //CvMat * sourceCv = cvLoadImageM(imgSource.c_str(), CV_LOAD_IMAGE_COLOR);
    CvMat * sourceCv = cvLoadImageM(imgSource.c_str(), CV_LOAD_IMAGE_GRAYSCALE);
    if (sourceCv == 0)
        return false;
    bool isMatched = false;
    for (int j=0; j!=n; ++j)
    {
        if (current == j) continue;
        string imgTarget = path + boost::lexical_cast<string>(setNumber) + "_" + boost::lexical_cast<string>(j+1) + ".jpg";
        //CvMat* target = cvLoadImageM(imgTarget.c_str(), CV_LOAD_IMAGE_COLOR);
        CvMat* target = cvLoadImageM(imgTarget.c_str(), CV_LOAD_IMAGE_GRAYSCALE);
        float cm = 0;
        if (target != 0)
            calcMatchingCoefficient4(sourceCv, target, fmtMatchPercentage, 0, 0, target->width, target->height-1, VerticalDivisionLimit, 0, 1, &cm);
        m(current, j) = cm;
        //m(j, current) = cm;
        if (!isMatched && cm != 0.0)
            isMatched = true;
     }
     return isMatched;
}

// returns matrix of matching coefficients between n images in setNumber set
Mat_<float> & Matcher::calcSetMatch (const int setNumber, const int fmtMatchPercentage, const int n, const string & path)
{
    Mat_<float> * result = new Mat_<float>(n, n, 0.0);
    int nMismatched = 0;
    for (int i=0; i!=n; ++i)
    {
        if (!matchWithOthers(*result, fmtMatchPercentage, setNumber, i, n, path))
            nMismatched += 1;
    }
    if (nMismatched > 2)
        std::cerr << "set " << setNumber << " has " << nMismatched << "mismatches!\n";
    return *result;
}
