#include "Classifier.h"

int Classifier::currentSet_ = StartSet;
pthread_mutex_t Classifier::mutex_ = PTHREAD_MUTEX_INITIALIZER;
string Classifier::outputName = "output.txt";
ofstream Classifier::output(Classifier::outputName.c_str(), ofstream::trunc);

//проверяем наличие значения х в векторе векторов v, ecли да, то сохраняем индекс векторов, в которых есть
bool Classifier::existsVV(int const x, vvi & v, vvi::iterator * t)
{
    for (vvi::iterator i = v.begin(); i!=v.end(); ++i)
        for (vector<int>::iterator j = i->begin(); j!=i->end(); ++j)
            if (*j == x)
            {
                if (t != 0)
                    *t = i;
                return true;
            }

    /*for (unsigned int i=0; i != v.size(); ++i)
        for (unsigned int j=0; j != v[i].size(); ++j)
            if (v[i][j] == x)
            {
                if (t != 0)
                    *t = i;
                return true;
            }*/
    return false;
}

void Classifier::append (int const i, vvi & sets, Mat_<float> const & m, map<int, vector<int> > & matches, vector<float> const & passValue)
{
    // вектор всех сетов, в которых лежат матчи i
    vector<vvi::iterator> correspondingSets;
    vvi::iterator t;
    for (unsigned int j = 0; j!= matches[i].size(); ++j)
    {
        // check if i's matches are already in sets
        if (existsVV(matches[i][j], sets, &t))
        {
            //check if match values correspond to passValues
            if (m(i, matches[i][j]) > passValue[i] && m(i, matches[i][j]) > passValue[matches[i][j]])
            {
                if (std::find(correspondingSets.begin(), correspondingSets.end(), t) == correspondingSets.end())
                    correspondingSets.push_back(t);
            }
        }
    }
    if (correspondingSets.empty())
    {
        // init a new set
        sets.push_back(vector<int>(1, i));
        //(*(sets.end()-1)).push_back(i);
        //sets[sets.size()-1].push_back(i);
        return;// false;
    }
    if (correspondingSets.size()==1)
    {
        correspondingSets[0]->push_back(i);
        return;// true;
    }
    // merge all corresponding sets
    sets.push_back(vector<int>());
    //int index = sets.size()-1;
    for (vector<vvi::iterator>::iterator it = correspondingSets.begin(); it != correspondingSets.end(); ++it)
    {
        for (vector<int>::iterator k=(*it)->begin(); k!=(*it)->end(); ++k)
        {
            (*sets.rbegin()).push_back(*k);
            //sets[sets.size()-1].push_back(*k);
        }
        sets.erase(*it);
    }
    (*sets.rbegin()).push_back(i);
    //return true;
}

bool Classifier::findOddImages (Mat_<float> const & m, const int setNumber)
{
    // img:vector of matching images
    map<int, vector<int> > matches;
    vector<float> passValue;
    for (int i = 0; i!=nImagesInSet; ++i)
    {
        matches.insert(std::pair<int, vector<int> >(i, vector<int>()));
        float passVal = 0;
        for (int j=0; j!=nImagesInSet; ++j)
            if (m(i, j)  > passVal)
                passVal = m(i, j);
        passVal *= matchLimit;
        passValue.push_back(passVal);
        if (passVal == 0) continue;
        for (int j=0; j!=nImagesInSet; ++j)
            if (i != j && m(i, j) > passVal)
                matches[i].push_back(j);
    }

    int elementWithMaxPassValue = 0;
    for (unsigned int i=1; i!=matches.size(); ++i)
        if (passValue[elementWithMaxPassValue] < passValue[i])
                elementWithMaxPassValue = i;

    if (passValue[elementWithMaxPassValue] == 0)
        return false;

    vvi sets;
    sets.push_back(matches[elementWithMaxPassValue]);
    (*sets.rbegin()).push_back(elementWithMaxPassValue);

    for (unsigned int i=0; i<matches.size(); ++i)
    {
        // смотрим, лежит ли i в каком-нибудь сете
        if (!existsVV(i, sets))
        {// если нет, то можно ли добавить ее к существующему сету
            append(i, sets, m, matches, passValue);
        }
    }

    // выбираем самый большой сет, остальные - лишние
    vvi::iterator maxSet = sets.begin();
    for (vvi::iterator it = sets.begin(); it!=sets.end(); ++it)
    {
        if ((*it).size() > (*maxSet).size())
            maxSet = it;
    }

    //если никто ни с кем не группировался
    if ((*maxSet).size()==1)
        return true; // false

    if ((*maxSet).size()==(unsigned int)nImagesInSet)
        return true;

    // если maxSet.size < половины фоток - значит, он лишний
    if ((*maxSet).size() < 1+nImagesInSet/2)
    {
        pthread_mutex_lock(&mutex_);
            if (!output.is_open())
                output.open(Classifier::outputName.c_str(), ofstream::app);

            for (vector<int>::iterator it = (*maxSet).begin(); it!=(*maxSet).end(); ++it)
                output << setNumber << "_" << (*it)+1 << ".jpg\n";

            output.close();
        pthread_mutex_unlock(&mutex_);
			
        return true;
    }

    pthread_mutex_lock(&mutex_);
    if (!output.is_open())
        output.open(Classifier::outputName.c_str(), ofstream::app);

    for (vvi::iterator iter = sets.begin(); iter != sets.end(); ++iter)
    {
        if (iter == maxSet)
            continue;
        for (vector<int>::iterator it = (*iter).begin(); it!=(*iter).end(); ++it)
            output << setNumber << "_" << (*it)+1 << ".jpg\n";

    }

    output.close();
    pthread_mutex_unlock(&mutex_);

    return true;
}

void Classifier::classify ()
{
    int from = -1;
    int nSets = -1;
    while (true)
    {
        pthread_mutex_lock(&mutex_);
        if (currentSet_ < FinishSet)
        {
            from = currentSet_;
            nSets = std::min(nSetsPerThread, FinishSet-from);
            currentSet_ += nSets;
        }
        pthread_mutex_unlock(&mutex_);

        if (from == -1)
            break;

        for (int i=from; i!=from+nSets; ++i)
        {
            std::cerr << i << " set\n";
            int nMismatched = 0;
            Mat_<float> m = stitcher.calcSetStitch2(i, &nMismatched);
            if (Print) printMat_(std::cerr, m);
            findOddImages(m, i);

            /*if (nMismatched > 2)
            {
                m = matcher.calcSetMatch(i);
                findOddImages(m, i);
            }
            else
                if (!findOddImages(m, i))
                {
                    m = matcher.calcSetMatch(i);
                    findOddImages(m, i);
                }

            if (Print) printMat_(std::cerr, m);*/
        }
        from = -1;
    }
}
