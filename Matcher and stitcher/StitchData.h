#ifndef STITCHDATA_H
#define STITCHDATA_H

#include <iostream>

struct StitchData {
    int nMatchPoints;
    double avgDistance;
    double matchPointsToTotalPoints;

    StitchData (int const nmp, double const ad, double const mpttp) : nMatchPoints(nmp), avgDistance(ad), matchPointsToTotalPoints(mpttp) {}

    StitchData () {
        nMatchPoints = 0;
        avgDistance = 0;
        matchPointsToTotalPoints = 0;
    }

    /*friend std::ostream &operator<< (ostream &stream, StitchData & sd)
        {
            stream << "(" << sd.nMatchPoints << "; " << sd.avgDistance << "; " << sd.matchPointsToTotalPoints << ") ";
            return stream;
        }*/
};


#endif // STITCHDATA_H
