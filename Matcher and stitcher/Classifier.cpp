#include "Classifier.h"

//проверяем наличие значения х в векторе v, если да, то возвращаем индекс найденного значения
bool Classifier::existsV (int const x, vector<int> v, int * k)
{
    for (unsigned int i=0; i!=v.size(); ++i)
        if (v[i]==x)
        {
            *k = i;
            return true;
        }
    return false;
}

//проверяем наличие значения х в векторе векторов v, ecли да, то возвращаем индекс вектора, в котором есть
bool Classifier::existsVV(int const x, vector<vector<int> > const & v, int * t)
{
    for (unsigned int i=0; i != v.size(); ++i)
        for (unsigned int j=0; j != v[i].size(); ++j)
            if (v[i][j] == x)
            {
                *t = i;
                return true;
            }
    return false;
}

// returnes index of new max set - to which we add image from set with size==1
int addToSet (vector<vector<int> > & sets, Mat_<float> const & m)
{
    int imageToAdd = -1;
    for (vector<vector<int> >::iterator i = sets.begin() ; i != sets.end(); ++i)
        if ((*i).size() ==1)
        {
            imageToAdd = (*i)[0];
            sets.erase(i);
            break;
        }

    float maxValue[2] = {-1, -1};
    int firstSetIndex = -1;
    int k = 0;
    int setToAddTo = -1;
    for (int i=0; i!=sets.size(); ++i)
    {
        if (firstSetIndex == -1)
            firstSetIndex = i;
        for (int j=0; j!=sets[i].size(); ++j)
        {
            if (m(imageToAdd, sets[i][j]) > maxValue[k])
            maxValue[k] = m(imageToAdd, sets[i][j]);
        }
        if (k == 0) {
            k += 1;
            continue;
        }
        setToAddTo = (maxValue[0] > maxValue[1]) ? firstSetIndex : i;
    }
    sets[setToAddTo].push_back(imageToAdd);
    return setToAddTo;
}

bool Classifier::findOddImages (Mat_<float> const & m, const int setNumber, ofstream& out)
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
        if (passVal == 0)
        {
            continue;
        }
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

    vector<vector<int> > sets;
    sets.push_back(matches[elementWithMaxPassValue]);
    sets[0].push_back(elementWithMaxPassValue);

    for (unsigned int i=0; i<matches.size(); ++i)
    {
        int t = -1;
        // смотрим, лежит ли i в каком-нибудь сете
        if (!existsVV(i, sets, &t))
        {// если нет, то лежат ли ее матчи в каком-нибудь сете
            bool was = false;
            // check if i's matches are already in sets
            for (unsigned int j = 0; j!=matches[i].size(); ++j)
            {
                if (existsVV(matches[i][j], sets, &t))
                {
                    //check if match values correspond to passValues
                    if (m(i, matches[i][j]) > passValue[i] && m(i, matches[i][j]) > passValue[matches[i][j]])
                    {
                        //проверить, есть ли эта картинка в матчах хотя бы 1 картинки из сета t

                       bool matches_ = false;
                        if (sets[t].size() == 1)
                            matches_ = true;
                        else
                            for (int q=0; q!=sets[t].size(); ++q)
                                if (std::find(matches[sets[t][q]].begin(), matches[sets[t][q]].end(), i) != matches[sets[t][q]].end())
                                    matches_ = true;

                        //bool matches_ = true;
                        if (matches_)
                        {
                            sets[t].push_back(i);
                            was = true;
                            break;
                        }
                    }
                }
            }
            if (!was)
            {// init a new set
                sets.push_back(vector<int>());
                sets[sets.size()-1].push_back(i);
            }
        }
    }

    // выбираем самый большой сет, остальные - лишние
    int maxSetIndex = 0;
    for (unsigned int i=1; i!=sets.size(); ++i)
        if (sets[i].size()>sets[maxSetIndex].size())
            maxSetIndex = i;

    //если никто ни с кем не группировался
    if (sets[maxSetIndex].size()==1)
        return false;

    if (sets[maxSetIndex].size()==2)
    {
        if (sets.size() == 3)
            // если есть сет с 1 картинкой, засунем ее куда-нибудь :)
            maxSetIndex = addToSet(sets, m);
        else
        {
            // этот сет лишний
            for (int i=0; i!=sets[maxSetIndex].size(); ++i)
                out << setNumber << "_" << sets[maxSetIndex][i]+1 << ".jpg\n";
            return true;
        }
    }

    for (int i=0; i!=sets.size(); ++i)
    {
        if (i==maxSetIndex)
            continue;
        for (int j=0; j!=sets[i].size(); ++j)
            out << setNumber << "_" << sets[i][j]+1 << ".jpg\n";
    }
    return true;
}

void Classifier::classify (const int from, const int to)
{
    // initialize output file
    ofstream output("output.txt", ofstream::trunc);
    if (!output.is_open())
    {
        std::cerr << "Cannot create output file.\n";
        return;
    }

    Stitcher stitcher;
    Matcher matcher;
    for(int i=from; i!=to; ++i)
    {
        std::cerr << i << " set\n";
        int nMismatched = 0;
        Mat_<float> m = stitcher.calcSetStitch2(i, &nMismatched);
        if (Print) printMat_(std::cerr, m);

        if (!output.is_open())
            output.open("output.txt", ofstream::app);

        /*if (nMismatched > 2)
        {
            m = matcher.calcSetMatch(i);
            findOddImages(m, i, output);
        }
        else*/
            if (!findOddImages(m, i, output))
            {
                m = matcher.calcSetMatch(i);
                findOddImages(m, i, output);
            }
        output.close();

        if (Print) printMat_(std::cerr, m);
    }
}
