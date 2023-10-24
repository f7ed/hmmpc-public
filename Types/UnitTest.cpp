#include "Types/UnitTest.h"
#include "Types/sfixMatrix.h"
#include "Protocols/PhaseConfig.h"
using namespace std;

namespace hmmpc
{

void testCint(PhaseConfig*phase)
{
    cint a(-2), b(-3);
    cout<<a+b<<endl;
    cout<<a-b<<endl;
    cout<<a*b<<endl;
    cout<<(-a)<<endl;
}

void testSfix(PhaseConfig*phase)
{
    sfix a(-4.6), b(2);
    cfix c(2.2), d(-1);
    
    a.input_from_party(0);
    b.input_from_party(1);
    // a.share() = a.secret();
    // b.share() = b.secret();
    cout<<"a:"<<a.reveal()<<endl;
    cout<<"b:"<<b.reveal()<<endl;
    cout<<"c:"<<c<<endl;
    cout<<"d:"<<d<<endl;

    phase->start_offline();
    phase->generate_truncated_random_sharings(5);
    phase->generate_reduced_truncated_sharings(5);
    phase->end_offline();

    phase->start_online();
    cout<<"a*b:"<<(a * b).reveal()<<endl;
    cout<<"a*c:"<<(a * c).reveal()<<endl;
    cout<<"b*d:"<<(b * d).reveal()<<endl;
    cout<<"c*d:"<<(c * d)<<endl;
    phase->end_online();
}

void debugSfixDivide(PhaseConfig*phase)
{
    // !Depricated
}

// UnitTests
void debugSintMatMul(PhaseConfig *phase)
{
    sintMatrix A(3, 3), B(3, 3);
    A.secret().setConstant(map_int_to_gfp(3));
    B.secret().setConstant(map_int_to_gfp(-2));

    // phase->generate_reduced_random_sharings(9+9);
    
    phase->start_online();
    A.input_from_party(0);
    B.input_from_party(1);
    cout<<"A:"<<endl<<A.reveal()<<endl;
    cout<<"B:"<<endl<<B.reveal()<<endl;
    cout<<"A*B:"<<endl<<(A * B).reveal()<<endl;
    cout<<"AxB:"<<endl<<(A.mult_cwise(B)).reveal()<<endl;
    phase->end_online();
}

// Test for efficiency on sint multiplication
void testSintMul(PhaseConfig *phase)
{
    const size_t N = 1000000;
    sintMatrix A(N, 1), B(N, 1);
    A.secret().setConstant(map_int_to_gfp(3));
    B.secret().setConstant(map_int_to_gfp(-2));

    phase->start_offline();
    phase->generate_reduced_random_sharings(N);
    phase->end_offline();

    A.input_from_party(0);
    B.input_from_party(1);

    phase->start_online();
    A.mult_cwise(B);
    phase->end_online();
}

// UnitTest for sfix matrix multiplication
void debugSfixMatMul(PhaseConfig *phase)
{
    cout<<"[UnitTest]:"<<endl;
    cout<<">>Mult between secret fixed-point numbers."<<endl<<endl;
    sfixMatrix A(2, 2), B(2, 2);
    RowMatrixXd a(2, 2);
    RowMatrixXd b(2, 2);
    a<<0.3, -0.004, 
       0.14, 0.05;
    b<<0.2, -1.2,
      -0.12, 0.06;

    cout<<"A:"<<endl<<a<<endl;
    cout<<"B:"<<endl<<b<<endl;

    map_float_to_gfp_matrix(a, A.secret());
    map_float_to_gfp_matrix(b, B.secret());
    
    A.input_from_party(0);
    B.input_from_party(1);

    phase->start_online();
    
    cout<<"A*B:(Matrix Mult)"<<endl<<(A * B).reveal()<<endl;
    cout<<"AxB:(Element-wise Mult)"<<endl<<(mult_cwise(A, B)).reveal()<<endl;
    // cout<<"A*B:"<<endl<<(A * B).reveal_quotient()<<endl;
    phase->end_online();
}

// UnitTest for multiplication between sfix and cfix (pure truncation)
void debugSfixMatMulCfix(PhaseConfig *phase)
{
    cout<<"[UnitTest]:"<<endl;
    cout<<">>Mult between secret fixed-point and public fixed-point, i.e. pure truncation."<<endl<<endl;
    sfixMatrix A(2, 2);
    cfixMatrix B(2, 2);
    RowMatrixXd a(2, 2);
    RowMatrixXd b(2, 2);
    a<<0.3, 0.1, 
       -0.2, 0.4;
    b<<2, -1,
      -0.1, -0.3;

    map_float_to_gfp_matrix(a, A.secret());
    map_float_to_gfp_matrix(b, B.values);
    
    cout<<"A:"<<endl<<a<<endl;
    cout<<"B:"<<endl<<b<<endl;
    A.input_from_party(0);

    phase->start_online();
    cout<<"AxB:(Element-wise Mult)"<<endl<<(A.mult_cwise_cfix(B)).reveal()<<endl;
    phase->end_online();
}

// Test for efficiency on sfix multiplication
void testSfixMul(PhaseConfig *phase)
{
    const size_t N = 1000000;
    sfixMatrix A(N, 1), B(N, 1);
    A.secret().setConstant(map_float_to_gfp(1.2));
    B.secret().setConstant(map_float_to_gfp(-2.4));

    phase->start_offline();
    phase->generate_reduced_truncated_sharings(N);
    phase->end_offline();

    A.input_from_party(0);
    B.input_from_party(1);

    phase->start_online();
    A.mult_cwise(B);
    phase->end_online();
}
}
