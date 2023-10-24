#ifndef TYPES_SINTMATRIX_H_
#define TYPES_SINTMATRIX_H_

#include "Types/sint.h"
#include "Types/cintMatrix.h"
#include "Protocols/ShareBundle.h"
#include "Protocols/PhaseConfig.h"
namespace hmmpc
{

class sintMatrix
{
    friend class sfixMatrix;
    friend sintMatrix operator+(const sintMatrix &a, const sintMatrix &b);
    friend sintMatrix operator-(const sintMatrix &a, const sintMatrix &b);
    friend sintMatrix operator*(const sintMatrix &a, const sintMatrix &b);
    friend sintMatrix mult_cwise(const sintMatrix &a, const sintMatrix &b);
    
protected:
    ShareBundle sharings;
public:
    sintMatrix(){}
    sintMatrix(const size_t &xSize, const size_t &ySize):sharings(xSize, ySize){}
    sintMatrix(const size_t &xSize, const size_t &ySize, const STYPE &x);
    sintMatrix(const gfpMatrix &_shares);
    sintMatrix(const ShareBundle &_sharings):sharings(_sharings){}

    // The reference to the shares.
    gfpMatrix& share(){gfpMatrix& ref = sharings.shares; return ref;}
    const gfpMatrix& share()const{const gfpMatrix &ref = sharings.shares; return ref;}
    gfpMatrix& secret(){gfpMatrix& ref = sharings.secrets; return ref;}
    const gfpMatrix& secret()const{const gfpMatrix &ref = sharings.secrets; return ref;}

    size_t rows()const{return share().rows();}
    size_t cols()const{return share().cols();}
    size_t size()const{return share().size();}

    // Input
    void input_from_file(int player_no);
    void input_from_party(int player_no);
    void input_from_random(int player_no);

    void input_from_file_request(int player_no);
    void input_from_party_request(int player_no);
    void input_from_random_request(int player_no);
    void finish_input_from(int player_no);


    // Reveal
    void reveal_to_party(int player_no);
    cintMatrix reveal();
    cintMatrix reveal(size_t num);

    sintMatrix& mult_cwise(const sintMatrix &other);

    sintMatrix& operator+=(const sintMatrix&other){share()+=other.share(); return *this;}
    sintMatrix& operator-=(const sintMatrix&other){share()-=other.share(); return *this;}

};

inline sintMatrix::sintMatrix(const size_t &xSize, const size_t &ySize, const STYPE &x):sharings(xSize, ySize)
{
    gfpScalar tmp = map_int_to_gfp(x);
    sharings.set_secret(tmp);
}

inline sintMatrix::sintMatrix(const gfpMatrix &_shares):sharings(_shares.rows(), _shares.cols())
{
    share() = _shares;
}

inline sintMatrix operator+(const sintMatrix&a, const sintMatrix &b) { return sintMatrix(a.share() + b.share());}
inline sintMatrix operator-(const sintMatrix&a, const sintMatrix &b) { return sintMatrix(a.share() - b.share());}
inline sintMatrix operator*(const sintMatrix&a, const sintMatrix&b)
{
    sintMatrix res(a.rows(), b.cols());
    res.share() = a.share() * b.share();
    res.sharings.reduce_degree();
    return res;
}

inline sintMatrix mult_cwise(const sintMatrix &a, const sintMatrix &b)
{
    sintMatrix res(a.rows(), a.cols());
    res.share() = a.share().array() * b.share().array();
    res.sharings.reduce_degree();
    return res;
}
/*********************************************
 *       Input Methods
 * *******************************************/
inline void sintMatrix::input_from_file(int player_no)
{
    if(ShareBase::P->my_num() == player_no){
        cintMatrix secrets(rows(), cols());
        ShareBase::in>>secrets;
        sharings.set_secrets(secrets.values);
    }
    sharings.input_from_party(player_no);
}

inline void sintMatrix::input_from_party(int player_no)
{
    sharings.input_from_party(player_no);
}

inline void sintMatrix::input_from_random(int player_no)
{
    sharings.input_from_random(player_no);
}

/*********************************************
 *       Input Methods of multi-thread version
 * *******************************************/

inline void sintMatrix::input_from_file_request(int player_no)
{
    if(ShareBase::P->my_num() == player_no){
        cintMatrix secrets(rows(), cols());
        ShareBase::in>>secrets;
        sharings.set_secrets(secrets.values);
    }
    sharings.input_from_party_request(player_no);
}

inline void sintMatrix::input_from_party_request(int player_no)
{
    sharings.input_from_party_request(player_no);
}

inline void sintMatrix::input_from_random_request(int player_no)
{
    sharings.input_from_random_request(player_no);
}

inline void sintMatrix::finish_input_from(int player_no)
{
    sharings.finish_input_from(player_no);
}

/*********************************************
 *       Reveal Methods
 * *******************************************/
inline void sintMatrix::reveal_to_party(int player_no)
{
    sharings.reveal_to_party(player_no);
    if(ShareBase::P->my_num()==player_no){
        cintMatrix res = sharings.get_secrets();
        cout<<"Output: "<<endl;
        cout<<res<<endl;
    }
}

inline cintMatrix sintMatrix::reveal()
{
    return cintMatrix(sharings.reveal());
}

inline cintMatrix sintMatrix::reveal(size_t num)
{
    sintMatrix tmp(1, num);
    tmp.share() = share().reshaped<Eigen::RowMajor>().head(num).transpose();
    return tmp.reveal();
}

inline sintMatrix& sintMatrix::mult_cwise(const sintMatrix &other)
{
    share() = share().array() * other.share().array();
    sharings.reduce_degree();
    return *this;
}

}
#endif