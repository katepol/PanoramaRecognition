#include <iostream>
#include <ctime>

#include "Classifier.h"
#include "Checker.h"
#include "Parameters.h"

#include <fstream>

#include <pthread.h>
#include <boost/shared_ptr.hpp>
typedef boost::shared_ptr <Classifier> ThreadPtr;         //smart pointer

using std::ofstream;

void initStitchLog()
{
    ofstream log("log_stitch_MT.txt", ofstream::trunc);
    if (!log.is_open())
    {
        cerr << "Cannot create log file.\n";
        return;
    }
    //log << "(StartSet, FinishSet,\nLastMinSquaredDistancePercent, HessianTreshold, CvSurfParamsExtended\nmatchLimit)\n";
    log.close();
}

void loggingStitch (time_t & t_start, float const mark = 0)
{
    ofstream log("log_stitch_MT.txt", ofstream::app);
    if (!log.is_open())
    {
        cerr << "Cannot open log file.\n";
        return;
    }
    time_t t_end;
    time(&t_end);

    log << "time per set " << (float)(t_end-t_start)/(float)(FinishSet-StartSet) << "s, nThreads = " << nThreads << ", nSetsPerThread = " << nSetsPerThread <<"; time elapsed "<< t_end-t_start << "s; "<< ctime(&t_end);

    log << StartSet << ", " << FinishSet << ", " << LastMinSquaredDistancePercent << ", " << HessianTreshold << ", " << CvSurfParamsExtended
            << ", " << matchLimit << "; mark = " << mark << "; time elapsed "<< t_end-t_start << "s; "<< ctime(&t_end)<<"\n";
    log.close();
}

void process ()
{
    ThreadPtr threads[nThreads];
    int index = 0;
    for(; index < nThreads; ++index)
    {
        ThreadPtr threadPtr(new Classifier());
        if(threadPtr->create() != 0)
        {
            index -= 1;
            continue;
        }
        threads[index] = threadPtr;
    }
      
    for(int i = 0; i < index; ++i) {
        threads[i]->join();
    }
}

int main()
{
    time_t time1, time2;
    time (&time1);
    std::cout << "start at " << ctime (&time1) << "\n";

    process();

    time (&time2);
    std::cout << "finish at " << ctime (&time2) << "\nprocessing results...";

    /*float mark = checkMultithreaded(Classifier::getOutputName(), learningFile);
    std::cout << "mark = " << mark;

    time (&time2);
    std::cout << "\n" << ctime (&time2);*/

    //initStitchLog();
    loggingStitch(time1);

    return 0;
}
