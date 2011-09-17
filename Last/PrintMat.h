#ifndef PRINTMAT_H
#define PRINTMAT_H

#include <fstream>
#include <opencv/cv.h>

template <typename T>
        void printMat_ (std::ostream & out, Mat_<T> const & m)
{
    for (int i=0; i!=m.rows; ++i)
    {
        for (int j=0; j!=m.cols; ++j)
            out << m(i, j) << " ; ";
        out << "\n";
    }
}

#endif // PRINTMAT_H
