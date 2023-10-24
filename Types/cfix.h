#ifndef TYPES_CFIX_H_
#define TYPES_CFIX_H_

#include "Math/gfpMatrix.h"
#include "Protocols/Share.h"
#include "Types/cint.h"

namespace hmmpc
{

class sfix;
class cfix:public cint
{
    friend istream& operator>>(istream &is, cfix &it);
    friend ostream& operator<<(ostream &os, const cfix &it);
    friend cfix operator+(const cfix &a, const cfix &b);
    friend cfix operator-(const cfix &a, const cfix &b);
    friend cfix operator*(const cfix &a, const cfix &b);
    friend cfix operator/(const cfix &a, const cfix &b);

    friend sfix operator+(const sfix &a, const cfix &b);
    friend sfix operator+(const cfix &a, const sfix &b);
    friend sfix operator-(const sfix &a, const cfix &b);
    friend sfix operator-(const cfix &a, const sfix &b);
    friend sfix operator*(const sfix &a, const cfix &b);
    friend sfix operator*(const cfix &a, const sfix &b);
public:
    cfix(){}
    cfix(const double &x):cfix(map_float_to_gfp(x)){}
    cfix(const cint &x):cfix(x.value){magnify();}
    cfix(const gfpScalar &_v):cint(_v){}
    cfix& truncate();
    cfix& magnify();
};

inline void test_fix(const double &obj, const cfix &res)
{
    cout<<"expected "<<obj<<", got "<<res<<endl;
}

inline istream& operator>>(istream &is, cfix &it)
{
    double x;
    is>>x;
    it.value = map_float_to_gfp(x);
    return is;
}

inline ostream& operator<<(ostream &os, const cfix &it)
{
    os<<map_gfp_to_float(it.value);
    return os;
}

// Truncate the last d bits.
inline cfix& cfix::truncate()
{
    value.truncate();
    return *this;
}

inline cfix& cfix::magnify()
{
    value.magnify();
    return *this;
}

inline cfix operator+(const cfix &a, const cfix &b) {return cfix(a.value + b.value);}
inline cfix operator-(const cfix &a, const cfix &b) {return cfix(a.value - b.value);}

inline cfix operator*(const cfix &a, const cfix &b)
{
    return cfix(a.value * b.value).truncate();
}

inline cfix operator/(const cfix &a, const cfix &b){
    return cfix(a.value / b.value).magnify();
}


}


#endif