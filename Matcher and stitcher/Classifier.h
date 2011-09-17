#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include <vector>
#include <fstream>
#include <map>

#include <QDebug>

#include "Parameters.h"
#include "Stitcher.h"
#include "Matcher.h"
#include "PrintMat.h"

using std::vector;
using std::ofstream;
using std::map;


class Classifier
{
    //проверяем наличие значения х в векторе v, если да, то возвращаем индекс найденного значения
    bool existsV (int const x, vector<int> v, int * k);
    //проверяем наличие значения х в векторе векторов v, ecли да, то возвращаем индекс вектора, в котором есть
    bool existsVV(int const x, vector<vector<int> > const & v, int * t);

    /*float getComparisonParameter (StitchMatrix const & m, int const i, int const j) {
        return (float)m(i, j).nMatchPoints;
    }

    float getComparisonParameter (Mat_<float> const & m, int const i, int const j) {
        return m(i, j);
    }*/

    bool findOddImages (Mat_<float> const & m, const int setNumber, ofstream& out);


public:
    Classifier() {}

    void classify (const int from = StartSet, const int to = FinishSet);
};

#endif // CLASSIFIER_H
