#ifndef TYPES_CINT_H_
#define TYPES_CINT_H_

#include "Math/gfpMatrix.h"
namespace hmmpc
{


class sint;
class cint
{

    friend sint operator+(const sint &a, const cint&b);
    friend sint operator+(const cint &a, const sint&b);
    friend sint operator-(const sint &a, const cint&b);
    friend sint operator-(const cint &a, const sint&b);
    friend sint operator*(const sint &a, const cint&b);
    friend sint operator*(const cint &a, const sint&b);
    friend sint operator/(const sint &a, const cint&b);
    friend sint operator/(const cint &a, const sint&b);

    friend istream& operator>>(istream &is, cint &it);
    friend ostream& operator<<(ostream &os, const cint &it);
    friend cint operator+(const cint &a, const cint &b);
    friend cint operator-(const cint &a, const cint &b);
    friend cint operator*(const cint &a, const cint &b);
    friend cint operator/(const cint &a, const cint &b);
    friend cint operator<(const cint &a, const cint &b);
    friend cint operator==(const cint&a, const cint &b);
    
public:
    gfpScalar value;
    cint(){}
    cint(const TYPE &x):value(map_int_to_gfp(x)){}
    cint(const gfpScalar &_v):value(_v){}
    bool is_negative()const{return value.get_value()>MAX_POSITIVE;}
    bool not_negative()const{return !is_negative();}

    cint operator-()const{return cint(-value);}
};


/************************************************************************
 * 
 *       Definition of cint
 * 
 * **********************************************************************/

inline void test_int(const STYPE &obj, const cint &res)
{
    cout<<"expected "<<obj<<", got "<<res<<endl;
}

inline istream& operator>>(istream &is, cint &it)
{
    STYPE tmp;
    is>>tmp;
    it.value = map_int_to_gfp(tmp);
    return is;
}

inline ostream& operator<<(ostream &os, const cint &it)
{
    os<<map_gfp_to_int(it.value);
    return os;
}

inline cint operator+(const cint &a, const cint &b) {return cint(a.value+b.value);}
inline cint operator-(const cint &a, const cint &b) {return cint(a.value-b.value);} 
inline cint operator*(const cint &a, const cint &b) {return cint(a.value*b.value);} 
inline cint operator/(const cint &a, const cint &b) {return cint(a.value/b.value);} 

inline cint operator<(const cint &a, const cint &b)
{
    // Need to negate the result if the sign bit is different
    bool flag = (a.is_negative() && b.is_negative()) || (a.not_negative() && b.not_negative());
    return cint((!flag) && (a.value < b.value));
}

inline cint operator==(const cint &a, const cint &b){return cint(a.value==b.value);}


}
#endif