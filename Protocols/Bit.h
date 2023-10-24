#ifndef PROTOCOLS_BIT_H_
#define PROTOCOLS_BIT_H_

#include "Protocols/ShareBundle.h"
#include "Protocols/PhaseConfig.h"
using Eigen::MatrixBase, Eigen::DenseBase;
namespace hmmpc
{

class Bit:public Share
{
public:
    Bit():Share(){}

    Bit& random();
};

class BitBundle:public ShareBundle
{
    // Bitwise xor on two sharings
    friend BitBundle bitwise_xor(const BitBundle &a, const BitBundle &b);
    // Bitwise xor beween a sharing and a public value
    template<typename Derived>
    friend BitBundle bitwise_xor(const BitBundle &a, const MatrixBase<Derived> &b);
    template<typename Derived>
    friend BitBundle bitwise_xor(const MatrixBase<Derived> &a, const BitBundle &b);
    
    // Bitwise and on two sharings
    friend BitBundle bitwise_and(const BitBundle &a, const BitBundle &b);
    // Bitwise xor beween a sharing and a public value
    template<typename Derived>
    friend BitBundle bitwise_and(const BitBundle &a, const MatrixBase<Derived> &b);
    template<typename Derived>
    friend BitBundle bitwise_and(const MatrixBase<Derived> &a, const BitBundle &b);

    // Unsigned less-than
    friend BitBundle less_than_unsigned(const BitBundle &a, const BitBundle &b);
    template<typename Derived>
    friend BitBundle less_than_unsigned(const MatrixBase<Derived> &a, const BitBundle &b);
    template<typename Derived>
    friend BitBundle less_than_unsigned(const BitBundle &a, const MatrixBase<Derived> &b);

    // Bit add
    friend BitBundle bit_add(const BitBundle &a, const BitBundle &b);
    template<typename Derived>
    friend BitBundle bit_add(const MatrixBase<Derived> &a, const BitBundle &b);
    template<typename Derived>
    friend BitBundle bit_add(const BitBundle &a, const MatrixBase<Derived> &b);
public:
    
    // Construction: Each row corresponds to one bitwise sharing.
    BitBundle(const size_t &n, const size_t &length):ShareBundle(n, length){}
    BitBundle(const size_t &n):ShareBundle(n, BITS_LENGTH){}
    BitBundle():ShareBundle(1, BITS_LENGTH){}// One bitwise sharing
    template <typename Derived>
    BitBundle(const Eigen::MatrixBase<Derived> &X){shares = X;}; // Stored in row-major
    // Private bitwise sharing
    BitBundle(const Share &x);
    BitBundle(const ShareBundle &X);

    // Random
    BitBundle& random();
    BitBundle& solved_random(Share &rField);//Get the corresponding t-sharing of the bits.
    BitBundle& solved_random(ShareBundle &rField);//Vectorization

    // Signed less-than
    BitBundle less_than(const BitBundle &other);

    // cond ? a : b
    ShareBundle if_else(const ShareBundle &a, const ShareBundle &b);

    // * Bit Op */
    // a AND b = ab (Hence, bit AND = multiplication)
    // a or b = a + b - ab
    // a XOR b = a + b - 2ab
    // Unbounded fan-in operation: OR, AND, XOR over each block. (evaluate the function value)
    BitBundle unbounded_blk_op(string fn, size_t blk_size);
    // Prefix op over large fan-in: we chunk it into small blocks, and use the prefix_blk_op to evalucate each block.
    BitBundle prefix_op(string fn, size_t blk_size = 8);
    BitBundle postfix_op(string fn, size_t blk_size = 8);
    // Prefix op on a small block using unbounded_blk_op.
    BitBundle prefix_blk_op(string fn, size_t blk_size = 8);
    BitBundle postfix_blk_op(string fn, size_t blk_size = 8); 

    BitBundle prefix_op_one_round(string fn);
    BitBundle postfix_op_one_round(string fn);

};

// This class is used to calculate the carry bits.
class TripleBitBundle{

    void represent_idx(TYPE idx, size_t mxBlk, vector<size_t> &rep);
public:
    BitBundle sBits;
    BitBundle pBits;
    BitBundle kBits;

    TripleBitBundle(const size_t &xSize, const size_t &ySize):sBits(xSize, ySize), pBits(xSize, ySize), kBits(xSize, ySize){}
    TripleBitBundle(const BitBundle &a, const BitBundle &b);
    template<typename Derived>
    TripleBitBundle(const BitBundle &a, const MatrixBase<Derived>&b);
    template<typename Derived>
    TripleBitBundle(const MatrixBase<Derived>&a, const BitBundle &b);

    size_t rows()const{return sBits.rows();}
    size_t cols()const{return sBits.cols();}
    TripleBitBundle unbounded_blk_carry_propagation(size_t blk_size);
    TripleBitBundle prefix_carry_propagation();
};

// Declaration of the templates
template<typename Derived>
BitBundle bitwise_xor(const BitBundle &a, const MatrixBase<Derived> &b);
template<typename Derived>
BitBundle bitwise_xor(const MatrixBase<Derived> &a, const BitBundle &b);

template<typename Derived>
BitBundle bitwise_and(const BitBundle &a, const MatrixBase<Derived> &b);
template<typename Derived>
BitBundle bitwise_and(const MatrixBase<Derived> &a, const BitBundle &b);

template<typename Derived>
BitBundle less_than_unsigned(const MatrixBase<Derived> &a, const BitBundle &b);
template<typename Derived>
BitBundle less_than_unsigned(const BitBundle &a, const MatrixBase<Derived> &b);

template<typename Derived>
BitBundle bit_add(const MatrixBase<Derived> &a, const BitBundle &b);
template<typename Derived>
BitBundle bit_add(const BitBundle &a, const MatrixBase<Derived> &b);
}

#endif