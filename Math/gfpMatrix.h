#ifndef MATH_GFP_MATRIX_H_
#define MATH_GFP_MATRIX_H_

#include "Math/gfpScalar.h"
#include "Tools/octetStream.h"
#include "Tools/random.h"
#include <immintrin.h>
using Eigen::MatrixBase, Eigen::RowMajor;
namespace hmmpc
{

// using block = __m128i;

// #ifdef __x86_64__
// __attribute__((target("sse2")))
// inline block makeBlock(uint64_t high, uint64_t low) {
// 	return _mm_set_epi64x(high, low);
// }
// #elif __aarch64__
// inline block makeBlock(uint64_t high, uint64_t low) {
// 	return (block)vcombine_u64((uint64x1_t)low, (uint64x1_t)high);
// }
// #endif

// const static block prs = makeBlock(2305843009213693951ULL, 2305843009213693951ULL);

typedef Eigen::Matrix<TYPE, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> RowMatrixXi64;
typedef Eigen::Matrix<TYPE, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor> ColMatrixXi64;
typedef Eigen::Matrix<TYPE, 1, Eigen::Dynamic, Eigen::RowMajor> RowVectorXi64;
typedef Eigen::Matrix<TYPE, Eigen::Dynamic, 1, Eigen::ColMajor> ColVectorXi64;
typedef Eigen::Matrix<STYPE, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> SignedRowMatrixXi64;
typedef Eigen::Matrix<STYPE, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor> SignedColMatrixXi64;
typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> RowMatrixXd;
typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor> ColMatrixXd;
typedef Eigen::Matrix<double, 1, Eigen::Dynamic, Eigen::RowMajor> RowVectorXd;
typedef Eigen::Matrix<double, Eigen::Dynamic, 1, Eigen::ColMajor> ColVectorXd;
/**
 * @brief Mod the matrix using SIMD.
 * 
 * @tparam Derived 
 * @param matrix 
 */
// template<typename Derived>
// void mod_matrix(Eigen::PlainObjectBase<Derived> &matrix)
// {
//     block *ptr = (block*)matrix.data();
//     for(size_t i = 0; i < matrix.size()/2; i++){
//         // 2-64bit (x & PR) + (x >> MERSENNE_PRIME_EXP);
//         ptr[i] = _mm_add_epi64(((block)ptr[i] & prs), _mm_srli_epi64(ptr[i], MERSENNE_PRIME_EXP));
//         // 2-64bit (i >= pv) ? i - pv : i;
//         ptr[i] = _mm_sub_epi64(ptr[i], _mm_andnot_si128(_mm_cmpgt_epi64(prs,ptr[i]), prs));
//     }

//     if(matrix.size() % 2){
//         size_t id_last = matrix.size() - 1;
//         matrix(id_last) = modPrime(matrix(id_last).get_value());
//     }
// }


/**
 * @brief There are two ways to generate a random matrix.
 * One is to call the modPrime() on each entry of the matrix.
 * The other is to generate the random memory directly, and mod them.
 * TODO [Test] For now, the performance is close.
 * @tparam Derived 
 * @param matrix 
 */
template<typename Derived>
void random_matrix(Eigen::PlainObjectBase<Derived> &matrix)
{   
    // * The first way: Generate random element one by one
    for(size_t i = 0; i < matrix.size(); i++){
        #if defined(PR_31) 
            matrix(i) = modPrime((unsigned int)secure_prng.get_uint());
        #elif defined(PR_61)
            matrix(i) = modPrime(secure_prng.get_word());
        #endif
    }

    // * The second way
    // size_t len = matrix.size()*sizeof(gfpScalar);
    // const auto *ptr = matrix.data();
    // secure_prng.get_octets((octet*)ptr, len);
    // mod_matrix(matrix);
}

template<typename Derived>
void random_matrix(Eigen::PlainObjectBase<Derived> &matrix, PRNG &prng)
{   
    // * The first way: Generate random element one by one
    for(size_t i = 0; i < matrix.size(); i++){
        #if defined(PR_31) 
            matrix(i) = modPrime((unsigned int)prng.get_uint());
        #elif defined(PR_61)
            matrix(i) = modPrime(prng.get_word());
        #endif
    }
}

/**
 * @brief Truncate each entry in the matrix.
 * 
 * @tparam Derived 
 * @param matrix 
 */
template<typename Derived>
void truncate_matrix(Eigen::MatrixBase<Derived> &matrix, size_t precision=FIXED_PRECISION)
{
    for(size_t i = 0; i < matrix.size(); i++){
        matrix(i).truncate(precision);
    }
}

/**
 * @brief Pack the matrix.row(i) into the octetStream os(i)
 * 
 * @param matrix [in]
 * @param os [out]
 */
template<typename Derived>
void pack_row(Eigen::PlainObjectBase<Derived> &matrix, octetStreams &os)
{
    assert(matrix.rows()==os.size());
    const auto *ptr = matrix.data();
    size_t size_row = matrix.cols() * sizeof(gfpScalar);
    for(size_t i = 0; i < matrix.rows(); i++){
        os[i].append((octet*)ptr, size_row);
        ptr += matrix.cols();
    }
}


/**
 * @brief Unpack the matrix.row(i) from the octetSream os(i)
 * 
 * @param matrix [out]
 * @param os [in]
 */
template<typename Derived>
void unpack_row(Eigen::PlainObjectBase<Derived> &matrix, octetStreams &os)
{
    assert(matrix.rows()==os.size());
    const auto *ptr = matrix.data();
    size_t size_row = matrix.cols() * sizeof(gfpScalar);
    for(size_t i = 0; i < matrix.rows(); i++){
        // In the reconstruction, os[P] is empty to avoid loopback.
        if(os[i].get_length()){
            os[i].consume((octet*)ptr, size_row);
        }
        ptr += matrix.cols();
    }
}

/**
 * @brief Pack the block of matrix into the octetStream.
 * 
 * @tparam Derived 
 * @param matrix [in]
 * @param startRow start row of the block
 * @param nRows number of rows in the block.
 * @param o [out]octetStream
 */
template<typename Derived>
void pack_rows(Eigen::PlainObjectBase<Derived> &matrix, const size_t &startRow, const size_t &nRows, octetStream &o)
{
    const auto *ptr = matrix.data();
    ptr += startRow * matrix.cols();
    o.append((octet*)ptr, nRows * matrix.cols() * sizeof(gfpScalar));
}

/**
 * @brief Unpack the octetStream to the corresponding block of the matrix.
 * 
 * @tparam Derived 
 * @param matrix [out]
 * @param startRow start row of the block
 * @param nRows number of rows in the block.
 * @param o [in]
 */
template<typename Derived>
void unpack_rows(Eigen::PlainObjectBase<Derived> &matrix, const size_t &startRow, const size_t &nRows, octetStream &o)
{
    const auto *ptr = matrix.data();
    ptr += startRow * matrix.cols();
    o.consume((octet*)ptr, nRows * matrix.cols() * sizeof(gfpScalar));
}

/**
 * @brief Pack several rows into os[i]. 
 * The partition is row-grained.
 * @tparam Derived 
 * @param matrix 
 * @param os 
 */
template<typename Derived>
void pack_rows(Eigen::PlainObjectBase<Derived> &matrix, octetStreams&os)
{
    assert(os.size());
    size_t n_rows = matrix.rows() / os.size();// Each block contains 'n_rows'
    size_t first_n_rows = matrix.rows() - n_rows * (os.size() - 1);// First block contains slightly more rows.
    const auto *ptr = matrix.data();

    // Pack the corresponding block into the octetStream.
    size_t first_n_gfp = first_n_rows * matrix.cols();
    size_t n_gfp = n_rows * matrix.cols();
    os[0].append((octet*)ptr, first_n_gfp * sizeof(gfpScalar));
    ptr += first_n_gfp;
    for(size_t i = 1; i < os.size(); i++){
        os[i].append((octet*)ptr, n_gfp * sizeof(gfpScalar));
        ptr += n_gfp;
    }
}


// Unpack the os[i] to the corresponding block of the matrix 
template<typename Derived>
void unpack_rows(Eigen::PlainObjectBase<Derived>&matrix, octetStreams&os)
{
    assert(os.size());
    size_t n_rows = matrix.rows() / os.size();
    size_t first_n_rows = matrix.rows() - n_rows * (os.size() - 1);
    const auto *ptr = matrix.data();

    size_t first_n_gfp = first_n_rows * matrix.cols();
    size_t n_gfp = n_rows * matrix.cols();
    
    if(os[0].get_length()){
        os[0].consume((octet*)ptr, first_n_gfp*sizeof(gfpScalar));
    }
    ptr += first_n_gfp;

    for(size_t i = 1; i < os.size(); i++){
        if(os[i].get_length()){
            os[i].consume((octet*)ptr, n_gfp*sizeof(gfpScalar));
        }
        ptr += n_gfp;
    }
}

/**
 * @brief Pack the whole matrix into the octetStream o
 * 
 * @param matrix [in]
 * @param o [out]
 */
template<typename Derived>
void pack_matrix(Eigen::PlainObjectBase<Derived> &matrix, octetStream &o)
{
    o.append((octet*)matrix.data(), matrix.size()*sizeof(gfpScalar));
}

/**
 * @brief Unpck the whole matrix from the octetStream o
 * 
 * @param matrix [out]
 * @param o [in]
 */
template<typename Derived>
void unpack_matrix(Eigen::PlainObjectBase<Derived> &matrix, octetStream &o)
{
    o.consume((octet*)matrix.data(), matrix.size()*sizeof(gfpScalar));
}

inline void getMSB_matrix(const gfpMatrix &input, gfpMatrix &res)
{
    for(size_t i = 0; i < input.size(); i++){
        res(i) = input(i).msb();
    }
}

template<typename Derived>
void decompose_bits(gfpScalar x, const size_t &bit_length, Eigen::MatrixBase<Derived>&res)
{
    assert(res.cols() == bit_length);
    assert(res.rows() == 1);
    res.setConstant(0);
    int idx = 0;
    while(!x.is_zero()){
        res(idx) = x&1;
        idx++;
        x >>= 1;
    }
}

template<typename Derived>
void decompose_bits(gfpVector x, const size_t &bit_length, Eigen::MatrixBase<Derived>&res)
{
    assert(res.cols() == bit_length);
    assert(res.rows() == x.size());
    res.setConstant(0);
    for(size_t i = 0; i < x.size(); i++){
        int idx = 0;
        while(!x(i).is_zero()){
            res(i, idx) = x(i)&1;
            idx++;
            x(i) >>= 1;
        }
    }
    
}

// Evaluate the prefix-products. (for batch inversion)
template<typename Derived>
void prefixMult(const Eigen::MatrixBase<Derived>&a, Eigen::MatrixBase<Derived>&res)
{
    assert(a.size() == res.size());
    res(0) = a(0);
    for(size_t i = 1; i < a.size(); i++){
        res(i) = res(i-1) * a(i);
    }
}

// Batch Inversion: Compute n inverse by 3(n-1) multiplications and a single inversion
inline void batch_inversion(const gfpMatrix&a, gfpMatrix&a_inv)
{
    assert(a.rows() == a_inv.rows());
    assert(a.cols() == a_inv.cols());
    gfpMatrix a_prefix_mult(a.rows(), a.cols());
    // p_i = p_i-1 * a_i
    prefixMult(a, a_prefix_mult);

    gfpMatrix a_prefix_inv(a.rows(), a.cols());
    gfpScalar inv_all = a_prefix_mult(Eigen::last);
    inv_all.inverse();
    // q_n = 1/p_n
    // q_i-1 = q_i * a_i
    a_prefix_inv(a.size()-1) = inv_all;
    for(size_t i = a.size()-1; i >=1; i--){
        a_prefix_inv(i-1) = a_prefix_inv(i) * a(i);
    }

    // a_inv_0 = q_0
    // a_inv_i = p_i-1 * q_i
    a_inv(0) = a_prefix_inv(0);
    a_inv.reshaped<Eigen::RowMajor>()(Eigen::seqN(1, a.size()-1)) = a_prefix_mult.reshaped<Eigen::RowMajor>()(Eigen::seqN(0, a.size()-1)).array() * 
                                                            a_prefix_inv.reshaped<Eigen::RowMajor>()(Eigen::seqN(1, a.size()-1)).array();
}

template<typename Derived>
const Eigen::Reshaped<const Derived>
reshape_helper(const Eigen::MatrixBase<Derived>& m, int rows, int cols)
{
  return Eigen::Reshaped<const Derived>(m.derived(), rows, cols);
}

//a is zero padded into b (initialized to zeros) with parameters as passed:
//imageWidth, imageHeight, padding, inputFilters, batchSize
template<typename Derived>
void zeroPad(const Eigen::MatrixBase<Derived>&a, Eigen::MatrixBase<Derived>&b,
            size_t iw, size_t ih, size_t P, size_t Din, size_t B)
{
    size_t size_B 	= (iw+2*P)*(ih+2*P)*Din;
	size_t size_Din = (iw+2*P)*(ih+2*P);
	size_t size_w 	= (iw+2*P);
    // rows() = the batchSize
    assert(a.rows() == B);
    assert(b.rows() == B);
    for(size_t i = 0; i < B; i++)
        for(size_t j = 0; j < Din; j++)
            for(size_t k = 0; k < ih; k++)
                for(size_t l = 0; l < iw; l++){
                    b(i, j*size_Din + (k+P)*size_w + l + P) = a(i, j*iw*ih + k*iw + l);
                }
}

// Extend a to b for convolution.
//imageWidth, imageHeight, outputWeight, outpuHeight, inputFilters, stepStride, filterSize, batchSize
// For image 2*2 with two features, the storage is feature1(2*2) feature2(2*2)
inline void convolExtend(const gfpMatrix &a, gfpMatrix&b,
                size_t iw, size_t ih, size_t ow, size_t oh,
                size_t Din, size_t S, size_t f, size_t B)
{
    assert(b.rows() == B*ow*oh);
    assert(b.cols() == f*f*Din);
    for(size_t i = 0; i < B; i++){
        // auto img = reshape_helper(a.row(i), ih, iw*Din);
        const gfpMatrix &img = a.row(i).reshaped<RowMajor>(ih*Din, iw);
        for(size_t j = 0; j < oh; j++)
            for(size_t k = 0; k < ow; k++){
                // for each feature
                for(size_t w = 0; w < Din; w++){
                    b.row(i*ow*oh + j*ow + k).segment(w*f*f, f*f) = img.block(w*ih+j*S, k*S, f, f).reshaped<RowMajor>(1, f*f);
                }
            }
    }
}

inline void maxpoolExtend(const gfpMatrix &a, gfpMatrix&b,
                size_t iw, size_t ih, size_t ow, size_t oh,
                size_t Din, size_t S, size_t f, size_t B)
{
    assert(b.rows() == B*ow*oh*Din);
    assert(b.cols() == f*f);
    for(size_t i = 0; i < B; i++){
        // auto img = reshape_helper(a.row(i), ih, iw*Din);
        const gfpMatrix &img = a.row(i).reshaped<RowMajor>(ih*Din, iw);
        for(size_t w = 0; w < Din; w++)// for each feature
            for(size_t j = 0; j < oh; j++)
                for(size_t k = 0; k < ow; k++){
                    // b.row(i*ow*oh + j*ow + k).segment(w*f*f, f*f) = img.block(w*ih+j*S, k*S, f, f).reshaped<RowMajor>(1, f*f);
                    b.row(i*ow*oh*Din + w*ow*oh + j*ow + k) = img.block(w*ih+j*S, k*S, f, f).reshaped<RowMajor>(1, f*f);
                }
            
    }
}

inline void print_oneline(const RowMatrixXd &matrix, std::string str)
{
#ifdef DEBUG_NN
    cout<<str<<matrix.reshaped<Eigen::RowMajor>(1, matrix.size())<<endl;
#endif
}

}

#endif