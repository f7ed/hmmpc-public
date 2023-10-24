#include "Math/UnitTest.h"

using namespace std;
namespace hmmpc
{
void testGfp()
{
    cout<<"[UnitTest for Gfp]:"<<endl;

    gfpScalar x=2;
    cout<<"x = "<<x<<endl;
    gfpScalar minus_x = -x;
    gfpScalar inverse_x = multiplicative_inverse((TYPE)(2));

    cout<<"-x = "<<minus_x<<endl;
    cout<<"x^{-1} = "<<inverse_x<<endl;
    cout<<"x+(-x)=" <<x+minus_x<<endl;
    cout<<"x*(x^-1) = "<<x*inverse_x<<endl;
}

void debugGfpDivision()
{
    // Not Used
}

void debugCNNExtend()
{
    gfpMatrix A(1,9*2);
    A<<1,2,3,4,5,6,7,8,9,
    11,12,13,14,15,16,17,18,19;
    cout<<A.reshaped<RowMajor>(6, 3)<<endl;
    gfpMatrix B(4, 8);
    convolExtend(A, B, 3, 3, 2, 2, 2, 1, 2, 1);
    cout<<B<<endl;
}
void debugMaxpoolExtend()
{
    gfpMatrix A(1,9*2);
    A<<1,2,3,4,5,6,7,8,9,
    11,12,13,14,15,16,17,18,19;
    cout<<A.reshaped<RowMajor>(6, 3)<<endl;
    
    gfpMatrix C(8, 4);
    maxpoolExtend(A, C, 3, 3, 2, 2, 2, 1, 2, 1);
    cout<<C<<endl;
}
}