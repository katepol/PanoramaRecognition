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

map<int, vector<int> > const & readOutput (string const & fileName)
{
    map<int, vector<int> > * odds = new map<int, vector<int> >(); //map: setNumber: vector of odd images

    ifstream file(fileName.c_str(), ifstream::in);
    if (!file.is_open())
    {
        cerr << "Cannot open "<< fileName <<"\n";
        return *odds;
    }

    string line = "";
    while (file.good())
    {
        file >> line;
        int setNum = getSetNum(line);
        map<int, vector<int> >::iterator it = odds->find(setNum);
        if (it == odds->end())
        {
            odds->insert(std::pair<int, vector<int> > (setNum, vector<int>(1, getImgNum(line))));
            continue;
        }
        it->second.push_back(getImgNum(line));
    }
    file.close();
    return *odds;
}

vector<int> const & getNextSet (int const setNum, map<int, vector<int> > const & odds)
{
    map<int, vector<int> >::const_iterator it = odds.find(setNum);
    if (it != odds.end())
        return it->second;

    return *(new vector<int>());
}

float checkMultithreaded (string const & myResult, string const & learning)
{
    ofstream cmpLog("compare_log.txt", ofstream::trunc);
    if (!cmpLog.is_open())
    {
        cerr << "Cannot create compare_log file.\n";
        return 0;
    }

    map<int, vector<int> > myOdds = readOutput(myResult);
    map<int, vector<int> > trueOdds = readOutput(learning);

    int currentSetN = StartSet;
    int n_wrong = 0;
    float mark = 0;

    Stitcher stitcher;

    while (currentSetN < FinishSet)
    {
        vector<int> v1 = getNextSet(currentSetN, myOdds);
        vector<int> v2 = getNextSet(currentSetN, trueOdds);
        float tmpMark = compareResults(v1, v2);
        if (tmpMark != 1.0)
        {
            cmpLog << currentSetN << "\nmy odds:";
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
    cmpLog << n_wrong << " classified wrong.";
    cmpLog.close();
    return (currentSetN-StartSet>0) ? mark/(currentSetN-StartSet) : mark;
}
