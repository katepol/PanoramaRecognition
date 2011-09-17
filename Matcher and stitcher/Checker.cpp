#include "Checker.h"


int getSetNum (string const & line)
{
    return boost::lexical_cast<int>(line.substr(0,line.find_first_of('_')));
}

int getImgNum (string const & line)
{
    return boost::lexical_cast<int>(line.substr(line.find_first_of('_')+1,1));
}

void readSet (vector<int> & v, ifstream & input, int const setN, string & s)
{
    v.clear();
    v.push_back(getImgNum(s));
    input >> s;
    while (getSetNum(s) == setN && input.good())
    {
        v.push_back(getImgNum(s));
        input >> s;
    }
}

bool contains (vector<int> const & v, int const k)
{
    return (std::find(v.begin(), v.end(), k) != v.end());
}

float compareResults (vector<int> const & my, vector<int> const & corr)
{
    if (corr.empty())
        return (nImagesInSet-my.size())/nImagesInSet;
    float mark = 0.0;
    bool inMy = false, inCorr = false;
    for (int i=0; i!=nImagesInSet; ++i)
    {
        inMy = contains(my, i+1);
        inCorr = contains(corr, i+1);
        if ((inMy && inCorr) || (!inMy && !inCorr))
            mark += 1/(float)nImagesInSet;
    }
    return mark;
}

void printIntVector (ofstream & out, vector<int> const & v) {
    if (v.empty())
    {
        out << " none\n";
        return;
    }
    for (unsigned int i=0; i!=v.size(); ++i)
        out << " " << v[i];
    out << "\n";
}

float checkResult (string const & myResult, string const & learning)
{
    int err1 = 0, err2 = 0;

    ifstream correct(learning.c_str(), ifstream::in);
    if (!correct.is_open())
    {
        cerr << "Cannot create learning file.\n";
        return 0;
    }
    ifstream my(myResult.c_str(), ifstream::in);
    if (!my.is_open())
    {
        cerr << "Cannot create myResult file.\n";
        return 0;
    }
    ofstream cmpLog("compare_log.txt", ofstream::trunc);
    if (!cmpLog.is_open())
    {
        cerr << "Cannot create compare_log file.\n";
        return 0;
    }

    float mark = 0.0;
    vector<int> v1, v2;
    int currentSetN = StartSet;
    string s1, s2;
    int n1 = 0, n2 = 0, n_wrong = 0;
    my>>s1;
    correct>>s2;

    Stitcher stitcher;

    while (my.good() && correct.good())
    {
        n1 = getSetNum(s1);
        n2 = getSetNum(s2);
        int k = std::min(n1, n2);

        if (k < StartSet)
        {
            if (n1 < StartSet)
                my>>s1;
            if (n2 < StartSet)
                correct>>s2;
            continue;
        }

        if (k!=currentSetN)
        {
            mark += k - currentSetN;
            currentSetN = k;
        }
        if (n1 == n2)
        {
            if (n1 != currentSetN)
            {
                cerr << "currentSetN != n2&&n1 :: " << currentSetN << " ; " << n2 << "\n";
            }
            readSet(v1, my, currentSetN, s1);
            readSet(v2, correct, currentSetN, s2);
        }
        else if (n1 > n2) // i have no odds, but they exist
        {
            if (n2 != currentSetN)
            {
                cerr << "currentSetN != n2 :: " << currentSetN << " ; " << n2 << "\n";
            }
            // read correct set
            v1.clear();
            readSet(v2, correct, currentSetN, s2);
            err1 +=1;
        }
        else // n2>n1 : i have odds, but there are no odds
        {
            if (n1 != currentSetN)
            {
                cerr << "currentSetN != n1 :: " << currentSetN << " ; " << n1 << "\n";
            }
            readSet(v1, my, currentSetN, s1);
            v2.clear();
            err2 +=1;
        }
        float tmpMark = compareResults(v1, v2);
        if (tmpMark != 1.0)
        {
            cmpLog << "set#" << currentSetN << " was classified wrong.\nmy odds:";
            printIntVector(cmpLog, v1);
            cmpLog << "true odds:";
            printIntVector(cmpLog, v2);

            int tmp;
            printMat_(cmpLog, stitcher.calcSetStitch2(currentSetN, &tmp));
            //Stitcher::printStitchMatrix(cmpLog, stitcher.calcSetStitch2(currentSetN));

            n_wrong += 1;

        }
        mark += tmpMark;
        currentSetN += 1;
    }
    cmpLog << "i have odds, but there are no odds = " <<err2 <<"\ni have no odds, but they exist = "<<err1 <<"\nWrong " << n_wrong << " of " << currentSetN-StartSet << "\n";

    cmpLog.close();
    my.close();
    correct.close();
    return (currentSetN-StartSet>0) ? mark/(currentSetN-StartSet) : mark;
}
