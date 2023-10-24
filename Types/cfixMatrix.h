#ifndef TYPES_CFIXMATRIX_H_
#define TYPES_CFIXMATRIX_H_

#include "Types/cfix.h"
#include "Types/cintMatrix.h"
#include <cmath>

#include<vector>
using std::vector;
namespace hmmpc
{

class cfixMatrix:public cintMatrix
{
    friend istream& operator>>(istream&is, cfixMatrix &it);
    friend ostream& operator<<(ostream&os, const cfixMatrix &it);
    friend cfixMatrix operator+(const cfixMatrix&a, const cfixMatrix &b);
    friend cfixMatrix operator-(const cfixMatrix&a, const cfixMatrix &b);
    friend cfixMatrix operator*(const cfixMatrix&a, const cfixMatrix &b);
public:
    cfixMatrix(){}
    cfixMatrix(const size_t &xSize, const size_t &ySize):cintMatrix(xSize, ySize){}
    cfixMatrix(const gfpMatrix &_v):cintMatrix(_v){}
    cfixMatrix& truncate();
    cfixMatrix& magnify();

    void input_rows(int startRow, int nRow);
    
    eMatrix<double> get_double();
};

inline istream& operator>>(istream &is, cfixMatrix &it)
{
    for(size_t i = 0; i < it.values.size(); i++)
    {
        cfix tmp;
        is>>tmp;
        it.values(i) = tmp.value;
    }
    return is;
}

inline ostream& operator<<(ostream &os, const cfixMatrix &it)
{
    eMatrix<double> res(it.values.rows(), it.values.cols());
    for(size_t i = 0; i < it.values.size(); i++){
        res(i) = map_gfp_to_float(it.values(i));
    }
    os<<res;
    return os;
}

inline cfixMatrix& cfixMatrix::truncate()
{
    for(size_t i = 0; i < values.size(); i++){
        values(i).truncate();
    }
    return *this;
}

inline cfixMatrix& cfixMatrix::magnify()
{
    for(size_t i = 0; i < values.size(); i++){
        values(i).magnify();
    }
    return *this;
}

inline cfixMatrix operator+(const cfixMatrix &a, const cfixMatrix &b) { return cfixMatrix(a.values+b.values);}
inline cfixMatrix operator-(const cfixMatrix &a, const cfixMatrix &b) { return cfixMatrix(a.values-b.values);}
inline cfixMatrix operator*(const cfixMatrix &a, const cfixMatrix &b)
{
    return cfixMatrix(a.values * b.values).truncate();
}

inline void cfixMatrix::input_rows(int startRow, int nRow)
{
    cfix tmp;
    for(size_t i = startRow; i < startRow + nRow; i++){
        for(size_t j = 0; j < cols(); j++){
            ShareBase::in>>tmp;
            values(i, j) = tmp.value;
        }
    }
}

inline eMatrix<double> cfixMatrix::get_double()
{
    eMatrix<double> res(rows(), cols());
    for(size_t i = 0; i < values.size(); i++){
        res(i) = map_gfp_to_float(values(i));
    }
    return res;
}

}
#endif