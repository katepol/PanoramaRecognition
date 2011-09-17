#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include <vector>
#include <fstream>
#include <map>
#include <list>
#include <pthread.h>

#include <QDebug>

#include "Parameters.h"
#include "Stitcher.h"
#include "PrintMat.h"

using std::vector;
using std::list;
using std::ofstream;
using std::map;

typedef list<vector<int> > vvi;

class Classifier
{
private:
    static pthread_mutex_t mutex_;
    static int currentSet_;
    static string outputName;
    static ofstream output;
    pthread_t myThread;

    Stitcher stitcher;

    //проверяем наличие значения х в векторе векторов v, ecли да, то возвращаем индекс вектора, в котором есть
    bool existsVV(int const x, vvi & v, vvi::iterator * t = 0);

    void append (int const i, vvi & sets, Mat_<float> const & m, map<int, vector<int> > & matches, vector<float> const & passValue);

    bool findOddImages (Mat_<float> const & m, const int setNumber);
    
    Classifier(Classifier const& other);                    //copy constructor
    Classifier& operator= (Classifier const& other);        //operator=
    
    static void* startRoutine(void* arg)
    {
        ((Classifier *) arg)->classify();
    }

public:
    Classifier() : stitcher(Stitcher()) {}

    void classify ();

    int create()
    {
        return pthread_create(&myThread, 0, Classifier::startRoutine, (void*) this);
    }
    int join()
    {
        return pthread_join(myThread, 0);
    }
    static string getOutputName()
    {
        return outputName;
    }
};




#endif // CLASSIFIER_H
