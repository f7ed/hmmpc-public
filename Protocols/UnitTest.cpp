#include "Protocols/PhaseConfig.h"
#include "Protocols/Bit.h"
namespace hmmpc
{
// Debug for Share generated with help of PRG.
void debugSharePRG(PhaseConfig *phase)
{
    Share a;
    a.set_secret(3);
    a.double_degree();

    DoubleShare b;

    phase->start_online();
    a.input_from_party_PRG(0);
    // a.input_from_party(0);
    b.input_from_random_PRG(2);
    cout<<"a: "<<a.get_secret()<<endl;
    cout<<"a-share: "<<a.share<<endl;
    cout<<"b-share: "<<b.share<<endl;
    cout<<"b-aux_share: "<<b.aux_share<<endl;
    phase->end_online();

    cout<<"a: "<<a.reveal()<<endl;
    cout<<"b: "<<b.reveal()<<endl;
    cout<<"b: "<<b.reveal_aux(ShareBase::threshold<<1)<<endl;
}

void debugShareBundlePRG(PhaseConfig *phase)
{
    ShareBundle A(3, 3);
    A.secret()<<1,2,3,4,5,6,7,8,9;

    DoubleShareBundle B(3, 3);

    phase->start_offline();
    phase->generate_random_sharings(1000);
    phase->end_offline();

    // cout<<"A:"<<endl<<A.secret()<<endl;
    phase->start_online();
    // A.input_from_party_PRG(0);

    octetStreams os_send;
    gfpMatrix shares_prng;
    octetStream o_rec;
    B.input_from_random_request_PRG(0, os_send, shares_prng, o_rec);
    B.input_aux_from_random_PRG(0);
    B.finish_input_from_PRG(0, os_send, shares_prng, o_rec);
    // B.input_from_random_PRG(0);

    // cout<<"A-shares:"<<endl<<A.shares<<endl;
    cout<<"B-shares:"<<endl<<B.shares<<endl;
    cout<<"B-aux_shares:"<<endl<<B.aux_shares<<endl;

    phase->end_online();
    // cout<<"A:"<<endl<<A.reveal()<<endl;
    cout<<"B:"<<endl<<B.reveal()<<endl;
    cout<<"B:"<<endl<<B.reveal_aux(ShareBase::threshold<<1)<<endl;
    
}

void debugRandomSharePRG(PhaseConfig *phase)
{
    phase->start_offline();
    phase->generate_reduced_random_sharings(10);
    phase->end_offline();

    phase->start_online();
    DoubleShareBundle A(3,3);
    A.reduced_random();
    cout<<"A:"<<A.reveal()<<endl;
    cout<<"A:"<<A.reveal_aux(ShareBase::threshold<<1)<<endl;
    phase->end_online();
}

// Debug for ShareBundle
void debugUnboundedPrefixMult(PhaseConfig *phase)
{
    ShareBundle A(3, 4);
    A.secret()<<1,2,3,4,
                2,2,2,2,
                5,5,5,5;
    
    A.input_from_party(0);

    phase->start_online();
    ShareBundle B = A.unbounded_prefix_mult();
    cout<<B.reveal()<<endl;
    phase->end_online();

    ShareBundle a(1, 4);
    a.secret()<<3,3,3,3;
    a.input_from_party(0);

    phase->start_online();
    ShareBundle b = a.unbounded_prefix_mult();
    cout<<b.reveal()<<endl;
    phase->end_online();
}

void debugGetLSB(PhaseConfig *phase)
{
    cout<<"[UnitTest]:"<<endl;
    cout<<">>Get LST"<<endl<<endl;

    cout<<"Input: (in bitwise)"<<endl;
    ShareBundle A(3,3);
    A.secret()<<1,2,3,4,5,6,7,8,9;
    
    cout<<"A:"<<endl<<A.secret()<<endl;
    A.input_from_party(0);

    phase->start_online();
    BitBundle lsb = A.get_LSB();
    cout<<"LSB:"<<endl<<lsb.reveal()<<endl;
    phase->end_online();
}

void debugGetMSB(PhaseConfig *phase)
{
    cout<<"[UnitTest]:"<<endl;
    cout<<">>Protocol 5.1: DReLU, i.e. getting MSB"<<endl<<endl;

    cout<<"Input: "<<endl;
    ShareBundle A(3,3);
    A.secret()<<0,2,PR-1,PR-2,5,PR-6,7,8,PR-1;
    
    cout<<"A:"<<endl<<A.secret()<<endl;
    A.input_from_party(0);

    phase->start_online();
    BitBundle msb = A.get_MSB();
    cout<<"MSB(sign bit):"<<endl<<msb.reveal()<<endl;
    phase->end_online();
}

void debugReLU(PhaseConfig *phase)
{
    cout<<"[UnitTest]:"<<endl;
    cout<<">>Protocol 5.3: ReLU"<<endl<<endl;

    cout<<"Input:"<<endl;
    ShareBundle A(3,3);
    A.secret()<<0,2,PR-1,PR-2,5,PR-6,7,0,((TYPE)1<<30)-1;


    cout<<"A:"<<endl<<A.secret()<<endl;
    A.input_from_party(0);
    ShareBundle reluPrime(3, 3), relu(3, 3);
    
    phase->start_online();
    
    // ReLU without optimization of 2-layer DN Multiplication;
    // cout<<"Without 2L-DN optimization"<<endl;
    // A.ReLU(reluPrime, relu);
    // cout<<"ReLU:"<<endl<<relu.reveal()<<endl;
    // cout<<"ReLUPrime:"<<endl<<reluPrime.reveal()<<endl;

    // *ReLU optimized with 2-layer DN Multiplication;
    cout<<endl<<"With 2L-DN optimization"<<endl;
    A.ReLU_opt(reluPrime, relu);
    // A.ReLU_opt_test(reluPrime, relu);
    cout<<"ReLU:"<<endl<<relu.reveal()<<endl;
    cout<<"ReLUPrime:"<<endl<<reluPrime.reveal()<<endl;

    phase->end_online();
}

void debugBoundPower(PhaseConfig *phase)
{
    // Not Used
}

void debugMaxPool(PhaseConfig *phase)
{
    cout<<"[UnitTest]:"<<endl;
    cout<<">>Protocol 5.4: Maxpool in rowwise"<<endl<<endl;

    cout<<"Input: "<<endl;
    ShareBundle A(3, 10);
    A.secret()<<1,2,3,1,2,3,1,2,3,1,
                0,4,1,1,2,3,1,2,3,1,
                6,2,3,1,2,3,9,2,3,1;

    cout<<"A:"<<endl<<A.secret()<<endl;
    A.input_from_party(0);

    phase->start_online();
    ShareBundle maxValue(3, 1), maxIdx(3, 10);

    // 10 rounds for one max

    // cout<<endl<<"(Maxpool without 2L-DN optimization)"<<endl;
    // A.MaxRowwise(maxValue, maxIdx);// 40 rounds for P0
    // A.MaxPrimeRowwise(maxIdx);

    // cout<<"max value:"<<endl<<maxValue.reveal()<<endl;
    // cout<<"max index:"<<endl<<maxIdx.reveal()<<endl;

    // *Maxpool with 2L-DN multiplication;
    // 8 rounds for one max
    cout<<endl<<"(Maxpool without 2L-DN optimization)"<<endl;
    A.MaxRowwise_opt(maxValue, maxIdx);// 32 rounds for P0
    cout<<"max value:"<<endl<<maxValue.reveal()<<endl;
    cout<<"max index:"<<endl<<maxIdx.reveal()<<endl;

    // 6 rounds for one maxRow
    A.MaxRowwise_opt(maxValue);
    cout<<"only max value:"<<endl<<maxValue.reveal()<<endl;
    phase->end_online();
}

// *Debug for bits
void debugBitOp(PhaseConfig *phase)
{
    cout<<"Bit-wise XOR:"<<endl;
    BitBundle A(3, 6), B(3, 7);
    gfpVector a(3), b(3), c(3);
    gfpMatrix c_bits(3, 6);
    a<<7,7,7;
    b<<15,15,15;
    c<<18,18,18;
    cout<<"a: "<<a.transpose()<<endl;
    cout<<"b: "<<b.transpose()<<endl;
    cout<<"c: "<<c.transpose()<<endl;

    decompose_bits(a, 6, A.secret());
    decompose_bits(b, 7, B.secret());
    decompose_bits(c, 6, c_bits);

    // phase->generate_reduced_random_sharings(3*6*2);
    // phase->setTrueOffline();

    A.input_from_party(0);
    B.input_from_party(1);

    phase->start_online();
    cout<<"a^b: "<<(7^15)<<endl;
    BitBundle res_xor1 = bitwise_xor(A, B);
    cout<<res_xor1.reveal()<<endl;

    cout<<"a^c: "<<(7^18)<<endl;
    BitBundle res_xor2 = bitwise_xor(A, c_bits);
    cout<<res_xor2.reveal()<<endl;

    cout<<"c^a: "<<(7^18)<<endl;
    BitBundle res_xor3 = bitwise_xor(c_bits, A);
    cout<<res_xor3.reveal()<<endl;

    cout<<"a&b: "<<(7&15)<<endl;
    BitBundle res_and1 = bitwise_and(A, B);
    cout<<res_and1.reveal()<<endl;

    cout<<"a&c: "<<(7&18)<<endl;
    BitBundle res_and2 = bitwise_and(A, c_bits);
    cout<<res_and2.reveal()<<endl;

    cout<<"c&a: "<<(7&18)<<endl;
    BitBundle res_and3 = bitwise_and(c_bits, A);
    cout<<res_and3.reveal()<<endl;
    phase->end_online();
}

void debugUnboundedOp(PhaseConfig *phase)
{
    // !Depricated
}

void debugPrefixBlkOp(PhaseConfig *phase)
{
    // !Depricated
}

void debugPrefixOp(PhaseConfig *phase)
{
    cout<<"[UnitTest]:"<<endl;
    cout<<">>Protocol 4.1: Prefix/Postfix - OR/AND."<<endl<<endl;
    // Prefix op on each row.
    cout<<"Input Bits: "<<endl;
    
    BitBundle A(4, 8);
    A.secret()<< 0,0,0,0,0,0,0,0,
                 1,1,0,0,0,0,0,0,
                 0,0,1,1,0,1,1,0,
                 1,1,1,1,1,1,1,1;
    cout<<A.secret()<<endl;

    A.input_from_party(1);
    phase->start_online();
    cout<<"Prefix OR:"<<endl<<A.prefix_op_one_round("OR").reveal()<<endl;
    cout<<"Prefix AND:"<<endl<<A.prefix_op_one_round("AND").reveal()<<endl;
    cout<<"Postfix OR:"<<endl<<A.postfix_op_one_round("OR").reveal()<<endl;
    cout<<"Postfix AND:"<<endl<<A.postfix_op_one_round("AND").reveal()<<endl;
    phase->end_online();
}

void debugLessThanUnsigned(PhaseConfig *phase)
{
    cout<<"[UnitTest]:"<<endl;
    cout<<">>Protocol 4.2: Bitwise-Less Than"<<endl<<endl;

    cout<<"Input: (in bitwise)"<<endl;
    BitBundle A(3), B(3);
    gfpVector a(3), b(3);
    a<<4,6,2;
    b<<1,7,2;
    
    decompose_bits(a, BITS_LENGTH, A.secret());
    decompose_bits(b, BITS_LENGTH, B.secret());

    cout<<"a: "<<endl<<a<<endl;
    cout<<"b: "<<endl<<b<<endl;

    A.input_from_party(0);
    B.input_from_party(1);
    
    phase->start_online();
    
    cout<<"A<B: (A and B are secretly shared)"<<endl<<less_than_unsigned(A, B).reveal()<<endl;
    cout<<"B<A: (A and B are secretly shared)"<<endl<<less_than_unsigned(B, A).reveal()<<endl;
    cout<<"a<B: (a is public)"<<endl<<less_than_unsigned(A.secret(), B).reveal()<<endl;
    cout<<"B<a: (a is public)"<<endl<<less_than_unsigned(B, A.secret()).reveal()<<endl; 
    phase->end_online();
}

void debugBitAdd(PhaseConfig *phase)
{
    // Not Used
}

void debugCarrayPropagation(PhaseConfig *phase)
{
    // Not Used
}

void debugBitsDecomposition(PhaseConfig *phase)
{
    // Not Used
}

}
