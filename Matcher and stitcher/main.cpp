#include <iostream>
#include <ctime>

#include "Classifier.h"
#include "Checker.h"
#include "Parameters.h"

#include <fstream>
using std::ofstream;

void initStitchLog()
{
    ofstream log("log_stitch.txt", ofstream::trunc);
    if (!log.is_open())
    {
        cerr << "Cannot create log file.\n";
        return;
    }
    log << "(StartSet, FinishSet,\nLastMinSquaredDistancePercent, HessianTreshold, CvSurfParamsExtended\nmatchLimit)\n";
    log.close();
}

void loggingStitch (float mark, time_t & t_start)
{
    ofstream log("log_stitch.txt", ofstream::app);
    if (!log.is_open())
    {
        cerr << "Cannot open log file.\n";
        return;
    }
    time_t t_end;
    time(&t_end);
    log << StartSet << ", " << FinishSet << ", " << LastMinSquaredDistancePercent << ", " << HessianTreshold << ", " << CvSurfParamsExtended
            << ", " << matchLimit << "; mark = " << mark << "; time elapsed "<< t_end-t_start << "s; "<< ctime(&t_end)<<"\n";
    log.close();
}

int main()
{
    time_t time1, time2;
    time (&time1);
    std::cout << "start at " << ctime (&time1) << "\n";

    Classifier classifier;
    classifier.classify();

    float mark = checkResult("output.txt", learningFile);
    std::cout << "mark = " << mark;

    time (&time2);
    time2 -= time1;
    std::cout << "\nTime elapsed " << time2 << "s\n";

    //initStitchLog();
    loggingStitch(mark, time1);

    return 0;
}
