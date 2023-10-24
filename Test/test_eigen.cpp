#include <Eigen/Core>
#include <iostream>
#include <fstream>
#include "Math/gfpScalar.h"
#include "Math/gfpMatrix.h"
#include "Tools/octetStream.h"
#include "Tools/time-func.h"
#include <emmintrin.h>
#include <cmath>
#include "Math/UnitTest.h"

using namespace std;
using namespace hmmpc;

using std::cout, std::endl;
using Eigen::last;
using Eigen::seq, Eigen::seqN, Eigen::RowMajor;
int main()
{
    cout<<"#threads: "<<Eigen::nbThreads()<<endl;
    
    // debugGfpDivision();
    // debugCNNExtend();
    // debugMaxpoolExtend();

    testGfp();
    
    return 0;
}