#ifndef TYPES_CINTMATRIX_H_
#define TYPES_CINTMATRIX_H_

#include "Types/cint.h"
namespace hmmpc
{

class cintMatrix
{
    friend istream& operator>>(istream &is, cintMatrix &it);
    friend ostream& operator<<(ostream &os, const cintMatrix &it);
    friend cintMatrix operator+(const cintMatrix &a, const cintMatrix &b);
    friend cintMatrix operator-(const cintMatrix &a, const cintMatrix &b);
    friend cintMatrix operator*(const cintMatrix &a, const cintMatrix &b);
public:
    gfpMatrix values;
    cintMatrix(){}
    cintMatrix(const size_t &xSize, const size_t &ySize):values(xSize, ySize){}
    cintMatrix(const gfpMatrix &_v):values(_v){}

    size_t rows()const{return values.rows();}
    size_t cols()const{return values.cols();}

    cintMatrix& relu(){values = (values.array() > MAX_POSITIVE).select(0, values); return*this;}
};

/************************************************************************
 * 
 *       Definition of cintMatrix
 * 
 * **********************************************************************/
inline istream& operator>>(istream&is, cintMatrix &it)
{
    for(size_t i = 0; i < it.values.size(); i++){
        cint tmp;
        is>>tmp;
        it.values(i) = tmp.value;
    }
    return is;
}

inline ostream& operator<<(ostream&os, const cintMatrix &it)
{
    eMatrix<STYPE> res(it.values.rows(), it.values.cols());
    for(size_t i = 0; i < it.values.size(); i++)
    {
        res(i) = map_gfp_to_int(it.values(i));
    }
    os<<res;
    return os;
}

inline cintMatrix operator+(const cintMatrix&a, const cintMatrix &b) {return cintMatrix(a.values+b.values);}
inline cintMatrix operator-(const cintMatrix&a, const cintMatrix &b) {return cintMatrix(a.values-b.values);}
inline cintMatrix operator*(const cintMatrix&a, const cintMatrix &b) {return cintMatrix(a.values*b.values);}

}
#endif