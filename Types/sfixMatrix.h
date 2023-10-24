#ifndef TYPES_SFIXMATRIX_H_
#define TYPES_SFIXMATRIX_H_
#include "Types/sintMatrix.h"
#include "Types/cfixMatrix.h"
#include "Protocols/PhaseConfig.h"
#include "Types/sfix.h"

namespace hmmpc
{

template<typename Derived>
void map_float_to_gfp_matrix(RowMatrixXd &X, Eigen::MatrixBase<Derived> &res)
{
    assert(X.rows()==res.rows());
    assert(X.cols()==res.cols());
    for(size_t i = 0; i < X.size(); i++){
        res(i) = map_float_to_gfp(X(i));
    }
}

class sfixMatrix:public sintMatrix
{
    friend sfixMatrix operator*(const sfixMatrix &a, const sfixMatrix &b);

    friend void cwiseMul(const sfixMatrix&a, const sfixMatrix&b, sfixMatrix &res);
    friend void cwiseMul(const sfixMatrix&a, const sfixMatrix&b, sfixMatrix &res, size_t precision);

    friend sfixMatrix mult(const sfixMatrix &a, const sfixMatrix &b, size_t precision);
    friend sfixMatrix mult_cwise(const sfixMatrix &a, const sfixMatrix &b);
    friend sfixMatrix mult_cwise(const sfixMatrix &a, const sfixMatrix &b, size_t precision);
    
public:
    sfixMatrix():sintMatrix(){}
    sfixMatrix(const size_t &xSize, const size_t &ySize):sintMatrix(xSize, ySize){}
    sfixMatrix(const size_t &xSize, const size_t &ySize, const double &x);
    sfixMatrix(const gfpMatrix &_shares):sintMatrix(_shares){}
    sfixMatrix(const ShareBundle &_sharings):sintMatrix(_sharings){}
    void resize(const size_t &xSize, const size_t &ySize){sharings.resize(xSize, ySize); sharings.set_degree(ShareBase::threshold);}
    // Input
    // void input_secret
    void input_from_file(int player_no);
    void input_secrets_from(string filename, int startRow, int nRow, double norm=1);
    void input_secrets_from_ColMajor(string filename, int startRow, int nRow, double norm=1);
    void distribute_shares();
    template<typename Derived>
    void fill_rows_from(int player_no, Eigen::MatrixBase<Derived> &matrix, size_t startRow, size_t nRow);

    // Reveal
    void reveal_to_party(int player_no);
    cfixMatrix reveal();
    cintMatrix reveal_quotient();
    cfixMatrix reveal(size_t num);
    cintMatrix reveal_quotient(size_t num);
    
    sfixMatrix& mult_cwise(const sfixMatrix &other);
    sfixMatrix& mult_cwise_cfix(const cfixMatrix &other);

    void reduce_degree(){sharings.reduce_degree();}
    void truncate(){sharings.truncate();}
    void truncate(size_t precision){sharings.truncate(precision);}
    // Reduce_truncate
    void reduce_truncate(){sharings.reduce_truncate();}
    void reduce_truncate(size_t precision){sharings.reduce_truncate(precision);}
    void reduce_truncate(vector<size_t> &precision){sharings.reduce_truncate(precision);}
    void reduce_truncate(size_t logLearningRate, size_t logMiniBatch){sharings.reduce_truncate(logLearningRate, logMiniBatch);}

    void partition_rows(size_t &startRow, size_t &nRow){sharings.partition_rows(startRow, nRow);}
    sfixMatrix& operator+=(const sfixMatrix &other){share()+=other.share(); return *this;}
    sfixMatrix& operator-=(const sfixMatrix &other){share()-=other.share(); return *this;}

    void ReLU(sintMatrix &deltaRelu, sfixMatrix &relu)const{sharings.ReLU(deltaRelu.sharings, relu.sharings);}
    void ReLU(sfixMatrix &relu)const{relu.sharings = sharings.ReLU();}
    void MaxpoolPrime(sintMatrix &maxPoolPrime)const{sharings.MaxPrimeRowwise(maxPoolPrime.sharings);}
    void Maxpool(sintMatrix &maxPrime, sfixMatrix &maxpool)const{sharings.MaxRowwise(maxpool.sharings, maxPrime.sharings);}
    void Maxpool(sfixMatrix &maxpool)const{sharings.MaxRowwise(maxpool.sharings);}// BUG LOG: Write MaxRowwise as Typo

    // Optimize by 2-layer mult
    void ReLU_opt(sintMatrix &deltaRelu, sfixMatrix &relu)const{sharings.ReLU_opt(deltaRelu.sharings, relu.sharings);}
    void ReLU_opt(sfixMatrix &relu)const{relu.sharings = sharings.ReLU_opt();}
    void Maxpool_opt(sfixMatrix &maxpool)const{sharings.MaxRowwise_opt(maxpool.sharings);}
    void Maxpool_opt(sintMatrix &maxPrime, sfixMatrix &maxpool)const{sharings.MaxRowwise_opt(maxpool.sharings, maxPrime.sharings);}
};

inline sfixMatrix::sfixMatrix(const size_t &xSize, const size_t &ySize, const double &x)
:sintMatrix(xSize, ySize)
{
    gfpScalar tmp = map_float_to_gfp(x);
    sharings.set_secret(tmp);
}

inline sfixMatrix operator+(const sfixMatrix&a, const sfixMatrix &b) {return sfixMatrix(a.share() + b.share());}
inline sfixMatrix operator-(const sfixMatrix&a, const sfixMatrix &b) {return sfixMatrix(a.share() - b.share());}
inline sfixMatrix operator*(const sfixMatrix &a, const sfixMatrix &b)
{
    sfixMatrix res(a.rows(), b.cols());
    res.share() = a.share() * b.share();
    res.sharings.reduce_truncate();
    return res;
}

inline sfixMatrix mult(const sfixMatrix &a, const sfixMatrix &b, size_t precision)
{
    sfixMatrix res(a.rows(), b.cols());
    res.share() = a.share() * b.share();
    res.sharings.reduce_truncate(precision);
    return res;
}

inline sfixMatrix mult_cwise(const sfixMatrix &a, const sfixMatrix &b)
{
    sfixMatrix res(a.rows(), a.cols());
    res.share() = a.share().array() * b.share().array();
    res.sharings.reduce_truncate();
    return res;
}

inline sfixMatrix mult_cwise(const sfixMatrix &a, const sfixMatrix &b, size_t precision)
{
    sfixMatrix res(a.rows(), a.cols());
    res.share() = a.share().array() * b.share().array();
    res.sharings.reduce_truncate(precision);
    return res; 
}

inline sfixMatrix divide(const sfixMatrix&a, const sfix&b)
{
    // VOID
}

// Each entry in b is the divider of each row in a.
inline sfixMatrix divideRowwise(const sfixMatrix &a, const sfixMatrix &b)
{
    // VOID
}   

inline void add(const sfixMatrix&a, const sfixMatrix&b, sfixMatrix&res){res.share() = a.share() + b.share();}
inline void substract(const sfixMatrix&a, const sfixMatrix&b, sfixMatrix&res){res.share() = a.share() - b.share();}
inline void cwiseMul(const sfixMatrix&a, const sfixMatrix&b, sfixMatrix&res){res.share() = a.share().array() * b.share().array(); res.reduce_truncate();}
inline void cwiseMul(const sfixMatrix&a, const sfixMatrix&b, sfixMatrix&res, size_t precision){res.share() = a.share().array()*b.share().array(); res.reduce_truncate(precision);}

/*********************************************
 *       Input Methods
 * *******************************************/
inline void sfixMatrix::input_from_file(int player_no)
{
    if(ShareBase::P->my_num()==player_no){
        cfixMatrix secrets(rows(), cols());
        ShareBase::in>>secrets;
        sharings.set_secrets(secrets.values);
    }
    sharings.input_from_party(player_no);
}

// Only input secrets without secret sharing.
inline void sfixMatrix::input_secrets_from(string filename, int startRow, int nRow, double norm)
{
    ifstream in(filename);
    for(int i = startRow; i < startRow+nRow; i++){
        for(int j = 0; j < cols(); j++){
            double tmp;
            in>>tmp;
            secret()(i, j) = map_float_to_gfp(tmp/norm);
        }
    }
    in.close();
}

// Input by column
inline void sfixMatrix::input_secrets_from_ColMajor(string filename, int startRow, int nRow, double norm)
{
    ifstream in(filename);
    for(int j = 0; j < cols(); j++){
        for(int i = startRow; i < startRow+nRow; i++){
            double tmp;
            in>>tmp;
            secret()(i, j) = map_float_to_gfp(tmp/norm);
        }
    }
    in.close();
}

inline void sfixMatrix::distribute_shares()
{
    // sharings.input_blocks_dispersed();
    sharings.input_blocks_dispersed_PRG();
}

template<typename Derived>
inline void sfixMatrix::fill_rows_from(int player_no, Eigen::MatrixBase<Derived> &matrix, size_t startRow, size_t nRow)
{
    if(ShareBase::P->my_num()==player_no){
        assert(matrix.rows() == nRow);
        assert(matrix.cols() == cols());
        for(size_t i = startRow, k = 0; i < startRow+nRow; i++, k++)
            for(size_t j = 0; j < cols(); j++){
                secret()(i, j) = map_float_to_gfp(matrix(k, j));
            }
    }
}

inline void sfixMatrix::reveal_to_party(int player_no)
{
    sharings.reveal_to_party(player_no);
    if(ShareBase::P->my_num()==player_no){
        cfixMatrix res = secret();
        cout<<"Output:"<<endl;
        cout<<res<<endl;
    }
}

inline cfixMatrix sfixMatrix::reveal()
{
    return cfixMatrix(sharings.reveal());
}

inline cfixMatrix sfixMatrix::reveal(size_t num)
{
    sfixMatrix tmp(1, num);
    tmp.share() = share().reshaped<Eigen::RowMajor>().head(num).transpose();
    return tmp.reveal();
}

inline cintMatrix sfixMatrix::reveal_quotient()
{
    return cintMatrix(sharings.reveal());
}

inline cintMatrix sfixMatrix::reveal_quotient(size_t num)
{
    sfixMatrix tmp(1, num);
    tmp.share() = share().reshaped<Eigen::RowMajor>().head(num).transpose();
    return tmp.reveal_quotient();
}

inline sfixMatrix& sfixMatrix::mult_cwise(const sfixMatrix&other)
{
    share() = share().array() * other.share().array();
    sharings.reduce_truncate();
    return *this;
}

inline sfixMatrix& sfixMatrix::mult_cwise_cfix(const cfixMatrix&other)
{
    share() = share().array() * other.values.array();
    sharings.truncate();
    return *this;
}

}
#endif