#include "Protocols/Bit.h"
#include "Protocols/RandomShare.h"
#include "Math/gfpMatrix.h"
#include <cmath>
using Eigen::seqN, Eigen::seq, Eigen::RowMajor, Eigen::last;
namespace hmmpc
{
/************************************************************************
 * 
 *       Definition of member functions about Bit
 * 
 * **********************************************************************/
Bit& Bit::random()
{
    RandomShare::get_random(RandomShare::queueRandomBit, share);
    return *this;
}
//************************ BitBundle **********************************

/**
 * @brief Decompose bits of each entry of a.
 * 
 * @param a colomn vector
 */
BitBundle::BitBundle(const ShareBundle &a):BitBundle(a.rows(), BITS_LENGTH)
{
    // Not Used
}

/************************************************************************
 * 
 *       Bitwise Functions about BitBundle
 * All functions are vectorized.
 * Note that the parameter BitBundle corresponds to the bitwise sharings,
 * and parameter MatrixBase<> corresponds to the public bits.
 * 
 * 1. bitwise_xor(a, b): a + b - 2ab
 * 2. bitwise_and(a, b): ab
 * 
 * **********************************************************************/
// ! Need to explicitly instantiate all the template instances.
// Op between bitwise sharings and public bits
template BitBundle bitwise_xor(const BitBundle &a, const MatrixBase<gfpMatrix> &b);
template BitBundle bitwise_xor(const MatrixBase<gfpMatrix> &a, const BitBundle &b);

template BitBundle bitwise_and(const BitBundle &a, const MatrixBase<gfpMatrix> &b);
template BitBundle bitwise_and(const MatrixBase<gfpMatrix> &a, const BitBundle &b);

// Bitwise xor between bitwise sharings and public bits.
// 1 round multiplication (assuming a.cols()<=b.cols())
BitBundle bitwise_xor(const BitBundle&a, const BitBundle &b)
{
    assert(a.rows() == b.rows());
    // Assume: a.cols()<=b.cols()
    if(b.cols()<a.cols()){return bitwise_xor(b, a);}

    BitBundle partRes(a.rows(), a.cols());
    partRes.shares = a.shares.array() * b.shares.array().leftCols(a.cols());
    partRes.reduce_degree();
    partRes.shares = a.shares + b.shares.leftCols(a.cols()) - (2 * partRes.shares);

    if(b.cols() == a.cols()){return partRes;}

    BitBundle res(a.rows(), b.cols());
    res.shares.leftCols(a.cols()) = partRes.shares;
    res.shares.rightCols(b.cols()-a.cols()) = b.shares.rightCols(b.cols()-a.cols());
    
    return res;
}

// (0 communication) Xor between the bitwise sharings and public bits.
template<typename Derived>
BitBundle bitwise_xor(const BitBundle&a, const MatrixBase<Derived> &b)
{
    assert(a.rows() == b.rows());
    assert(a.cols() == b.cols());

    BitBundle res(a.rows(), a.cols());
    res.shares = a.shares.array() + b.array() - (2 * a.shares.array() * b.array());
    return res;
}

template<typename Derived>
BitBundle bitwise_xor(const MatrixBase<Derived> &a, const BitBundle &b)
{
    return bitwise_xor(b, a);
}

BitBundle bitwise_and(const BitBundle&a, const BitBundle &b)
{
    assert(a.rows() == b.rows());
    // Assume: a.cols()<=b.cols()
    if(b.cols()<a.cols()){return bitwise_and(b, a);}

    BitBundle partRes(a.rows(), a.cols());
    partRes.shares = a.shares.array() * b.shares.array().leftCols(a.cols());
    partRes.reduce_degree();

    if(b.cols() == a.cols()){return partRes;}

    BitBundle res(a.rows(), b.cols());
    res.shares.leftCols(a.cols()) = partRes.shares;
    res.shares.rightCols(b.cols()-a.cols()) = b.shares.rightCols(b.cols()-a.cols());
    
    return res;
}

template<typename Derived>
BitBundle bitwise_and(const BitBundle &a, const MatrixBase<Derived> &b)
{
    assert(a.rows() == b.rows());
    assert(a.cols() == b.cols());

    BitBundle res(a.rows(), a.cols());
    res.shares = a.shares.array() * b.array();
    return res;
}

template<typename Derived>
BitBundle bitwise_and(const MatrixBase<Derived> &a, const BitBundle &b)
{
    return bitwise_and(b, a);
}

/************************************************************************
 * 
 *       Functionalities about BitBundle (could be of different length)
 * All functions are vectorized.
 * Note that the parameter BitBundle corresponds to the bitwise sharings,
 * and parameter MatrixBase<> corresponds to the public bits.
 * 
 * 1. less_than_unsigned (a, b): a < b
 * 2. bitwise_and(a, b): ab
 * 
 * **********************************************************************/

// Explicitly instantiate all the template instances
template BitBundle less_than_unsigned(const MatrixBase<gfpMatrix> &a, const BitBundle &b);
template BitBundle less_than_unsigned(const BitBundle &a, const MatrixBase<gfpMatrix> &b);

template BitBundle bit_add(const MatrixBase<gfpMatrix> &a, const BitBundle &b);
template BitBundle bit_add(const BitBundle &a, const MatrixBase<gfpMatrix> &b);


/**
 * @brief Functionality of less than between two bitwise sharing.
 * Note that it saves 2 rounds of MULT when b is public. 
 * @param a 
 * @param b 
 * @return BitBundle 
 */
BitBundle less_than_unsigned(const BitBundle &a, const BitBundle &b)
{
    // a and b can be of different size
    BitBundle xorRes = bitwise_xor(a, b);

    // Postfix-OR of xorRes
    BitBundle postfixOr(xorRes.rows(), xorRes.cols());
    postfixOr.shares = xorRes.postfix_op_one_round("OR").shares;

    size_t len = xorRes.cols();

    // Evaluate the delta of postfix.
    BitBundle deltaXor(a.rows(), len);
    deltaXor.shares.col(len-1) = postfixOr.shares.col(len-1);
    for(size_t i = 0; i < len-1; i++){
        deltaXor.shares.col(i) = postfixOr.shares.col(i) - postfixOr.shares.col(i+1);
    }

    BitBundle res(a.rows(), 1);
    // res.row(i) = inner product of deltaXor.row(i) and b.row(i)
    for(size_t i = 0; i < a.rows(); i++){
        res.shares.row(i) = deltaXor.shares.row(i)(seqN(0, b.cols())) * b.shares.row(i).transpose();
    }
    res.reduce_degree();
    return res;
}

// Time = 2 round
template<typename Derived>
BitBundle less_than_unsigned(const MatrixBase<Derived> &a, const BitBundle &b)
{
    // a and b can be of different size
    BitBundle xorRes = bitwise_xor(a, b);

    // Postfix-OR of xorRes
    BitBundle postfixOr(xorRes.rows(), xorRes.cols());
    postfixOr.shares = xorRes.postfix_op_one_round("OR").shares;

    size_t len = xorRes.cols();

    BitBundle deltaXor(a.rows(), len);
    deltaXor.shares.col(len-1) = postfixOr.shares.col(len-1);
    for(size_t i = 0; i < len-1; i++){
        deltaXor.shares.col(i) = postfixOr.shares.col(i) - postfixOr.shares.col(i+1);
    }

    BitBundle res(a.rows(), 1);
    res.shares.setConstant(0);
    for(size_t i = 0; i < a.rows(); i++){
        res.shares.row(i) = deltaXor.shares.row(i)(seqN(0, b.cols())) * b.shares.row(i).transpose();
    }
    res.reduce_degree();
    return res;
}

// Time = 1 round
template<typename Derived>
BitBundle less_than_unsigned(const BitBundle &a, const MatrixBase<Derived> &b)
{
    BitBundle xorRes = bitwise_xor(a, b);

    // Postfix-OR of xorRes
    BitBundle postfixOr(xorRes.rows(), xorRes.cols());
    postfixOr.shares = xorRes.postfix_op_one_round("OR").shares;

    size_t len = xorRes.cols();

    BitBundle deltaXor(a.rows(), len);
    deltaXor.shares.col(len-1) = postfixOr.shares.col(len-1);
    for(size_t i = 0; i < len-1; i++){
        deltaXor.shares.col(i) = postfixOr.shares.col(i) - postfixOr.shares.col(i+1);
    }

    BitBundle res(a.rows(), 1);
    res.shares.setConstant(0);
    for(size_t i = 0; i < a.rows(); i++){
        res.shares.row(i) = deltaXor.shares.row(i)(seqN(0, b.cols())) * b.row(i).transpose();
    }
    return res;
}

/**
 * @brief Bit add on two bitwise sharings.
 * 
 * @param a 
 * @param b 
 * @return BitBundle The length of result is increased by 1.
 */
BitBundle bit_add(const BitBundle&a, const BitBundle &b)
{
    // Not Used
}

template<typename Derived>
BitBundle bit_add(const BitBundle &a, const MatrixBase<Derived>&b)
{
    // Not Used
}

template<typename Derived>
BitBundle bit_add(const MatrixBase<Derived>&a, const BitBundle &b)
{
    return bit_add(b, a);
}
/************************************************************************
 * 
 *       Definition of member functions about BitBundle
 * 
 * **********************************************************************/
// Random bitwise sharings.
BitBundle& BitBundle::random()
{
#ifdef ZERO_OFFLINE
    shares.setConstant(0);
    return *this;
#endif

    if(!Phase->is_true_offline()){
        if(Phase->is_Offline()){
            Phase->generate_random_bits(size());
        }
        else{
            Phase->switch_to_offline();
            Phase->generate_random_bits(size());
            Phase->switch_to_online();
        }
    }
    RandomShare::get_randoms(RandomShare::queueRandomBit, shares);
    return *this;
}

BitBundle& BitBundle::solved_random(Share &rField)
{
    assert(rows()==1);
    random();
    if(Phase->is_Offline()){
        rField.share = (shares * bits_coeff)(0,0);
    }else{
        Phase->switch_to_offline();
        rField.share = (shares * bits_coeff)(0,0);
        Phase->switch_to_online();
    }
    return *this;
}

// Get random bitwise sharings and corresponding solved sharings in Fp.
BitBundle& BitBundle::solved_random(ShareBundle &rField)
{
    random();
#ifdef ZERO_OFFLINE
    rField.shares.setConstant(0);
    return *this;
#endif

    if(Phase->is_Offline()){
        rField.shares = (shares * bits_coeff).reshaped<RowMajor>(rField.rows(), rField.cols());
    }else{
        Phase->switch_to_offline();
        rField.shares = (shares * bits_coeff).reshaped<RowMajor>(rField.rows(), rField.cols());
        Phase->switch_to_online();
    }
    return *this;
}

// cond ? a : b on each entry
ShareBundle BitBundle::if_else(const ShareBundle&a, const ShareBundle &b)
{
    ShareBundle res(a.rows(), a.cols());
    res.shares = shares.array() * a.shares.array() + (1 - shares.array()) * b.shares.array();
    res.reduce_degree();
    return res;
}

/**
 * @brief Unbounded fan-in op of each block.
 * We split the blocks vertically, and calculate the op result of each block.
 * Op includes OR, AND, XOR.
 * 
 * @param blk_size 
 * @return BitBundle 
 */
BitBundle BitBundle::unbounded_blk_op(string fn, size_t blk_size)
{
    // !Depricated
}

/**
 * @brief Prefix op on a small block.
 * We construct a new matrix, and call unbounded_blk_op.
 * 
 * * There are two cases.
 * * 1. If cols() > blk_size, we need to split into blocks on row-wise (or vertically). Then call 2.
 * * 2. If cols() == blk_size, we contruct an extend matrix to call unbounded_blk_op.
 * @param fn 
 * @param blk_size We deal each block on parallel, which means to calculate the prefix op on each block.
 * @return BitBundle Each block corresponds to the prefix op on the input block.
 */
BitBundle BitBundle::prefix_blk_op(string fn, size_t blk_size)
{   
    // !Depricated
}

BitBundle BitBundle::postfix_blk_op(string fn, size_t blk_size)
{
    // Depricated
}

/**
 * @brief Prefix op over large fan-in.
 * We chunk each row into small blocks, and use the prefix_blk_op to evalucate each block.
 * 
 * @param phase If in true offline, we use the unbounded mult random sharings in the queue, and generate in separate offline otherwise.
 * @param fn 
 * @param blkSize 
 * @return BitBundle 
 */
BitBundle BitBundle::prefix_op(string fn, size_t blkSize)
{
    // !Depricated
}

BitBundle BitBundle::postfix_op(string fn, size_t blk_size)
{
    // !Depricated
}

BitBundle BitBundle::prefix_op_one_round(string fn)
{
    BitBundle res(rows(), cols());
    if(fn=="AND"){
        res.shares = unbounded_prefix_mult().shares;
        return res;
    }

    BitBundle mapped(rows(), cols());
    if(fn=="OR"){
        mapped.shares = 1 - shares.array();
        res.shares = 1 - mapped.unbounded_prefix_mult().shares.array();
        return res;
    }
}

BitBundle BitBundle::postfix_op_one_round(string fn)
{
    BitBundle res(rows(), cols());
    if(fn=="AND"){
        res.shares = unbounded_postfix_mult().shares;
        return res;
    }

    BitBundle mapped(rows(), cols());
    if(fn=="OR"){
        mapped.shares = 1 - shares.array();
        res.shares = 1 - mapped.unbounded_postfix_mult().shares.array();
        return res;
    }
}

/************************************************************************
 * 
 *       Definition of member functions about TripleBitBundle
 * 
 * **********************************************************************/
template TripleBitBundle::TripleBitBundle(const BitBundle &a, const MatrixBase<gfpVector>&b);
template TripleBitBundle::TripleBitBundle(const BitBundle &a, const MatrixBase<gfpMatrix>&b);
template TripleBitBundle::TripleBitBundle(const MatrixBase<gfpVector>&a, const BitBundle &b);
template TripleBitBundle::TripleBitBundle(const MatrixBase<gfpMatrix>&a, const BitBundle &b);
TripleBitBundle::TripleBitBundle(const BitBundle &a, const BitBundle &b)
{
    // Not Used
}

template<typename Derived>
TripleBitBundle::TripleBitBundle(const BitBundle &a, const MatrixBase<Derived> &b):sBits(a.rows(), a.cols()), pBits(a.rows(), a.cols()), kBits(a.rows(), a.cols())
{
    // Not Used
}

template<typename Derived>
TripleBitBundle::TripleBitBundle(const MatrixBase<Derived> &a, const BitBundle &b):TripleBitBundle(b, a){}

TripleBitBundle TripleBitBundle::unbounded_blk_carry_propagation(size_t blkSize)
{
    // Not Used
}

void TripleBitBundle::represent_idx(TYPE idx, size_t mxBlkSize, vector<size_t> &rep)
{
    // Not Used
}

/**
 * @brief Prefix carry propagation.
 * 
 * @return TripleBitBundle sBit is the carry bit.
 */
TripleBitBundle TripleBitBundle::prefix_carry_propagation()
{
    // Not Used
}
}