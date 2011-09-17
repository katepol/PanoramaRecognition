#ifndef CHECKER_H
#define CHECKER_H

#include <algorithm> //for min
#include <cstring>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <vector>
#include <iostream>
#include <map>

#include "Parameters.h"
#include "Stitcher.h"
#include "PrintMat.h"

using std::string;
using std::ifstream;
using std::ofstream;
using std::vector;
using std::cerr;
using std::map;

//float checkResult (string const & myResult, string const & learning);

float checkMultithreaded (string const & myResult, string const & learning);

#endif // CHECKER_H
