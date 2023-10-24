#ifndef MATH_GFP_SCALAR_H_
#define MATH_GFP_SCALAR_H_

// TYPE = {PR_31, PR_61}
#define PR_31

#include <iostream>
#include <Eigen/Core>
#include "Tools/random.h"

namespace hmmpc
{

#if defined(PR_31) && !defined(PR_61)
using TYPE = unsigned int;//{unsigned int, uint64_t}
using DTYPE = uint64_t;//{uint64_t, __uint128_t}
using STYPE = int;//{int, int64_t}
using u64 = uint64_t;
// {31, 61, 127}
const int MERSENNE_PRIME_EXP=31;
const int PRIME_LENGTH = MERSENNE_PRIME_EXP;
const static TYPE PR = 2147483647;
const static DTYPE pr = 2147483647;

const static TYPE RSQRT_CONST = 1610612734; // rsqrt = x^{RSQRT_CONST} = 1/sqrt(x)

// Bit
const static size_t BITS_LENGTH = 31; // {31, 61}
const static size_t n_BITS_LENGTH = 5; // {5, 6}6 = log(61)+1
const static size_t FIXED_PRECISION =12; // 11 bits for fixed point part

const static TYPE ConstTwoInverse = 1073741824;

#elif defined(PR_61) && !defined(PR_31)
using TYPE = uint64_t;//{unsigned int, uint64_t}
using DTYPE = __uint128_t;//{uint64_t, __uint128_t}
using STYPE = int64_t;//{int, int64_t}
using u64 = uint64_t;
const int MERSENNE_PRIME_EXP=61;
const int PRIME_LENGTH = MERSENNE_PRIME_EXP;
const static TYPE PR = 2305843009213693951;
const static DTYPE pr = 2305843009213693951;

const static TYPE RSQRT_CONST = 1729382256910270462; // rsqrt = x^{RSQRT_CONST} = 1/sqrt(x)

// Bit
const static size_t BITS_LENGTH = 61; // {31, 61}
const static size_t n_BITS_LENGTH = 6; // {5, 6}6 = log(61)+1
const static size_t FIXED_PRECISION = 13; // 21 bits for fixed point part

const static TYPE ConstTwoInverse = 1152921504606846976;
#endif

static SeededPRNG secure_prng; // PRNG
const static TYPE MID_PR = ((PR-1)>>1) +1; // Middle point

const static TYPE SQRT_EXP = (PR+1)>>2; // sqrt(x) = x^{SQRT_EXP}
const static TYPE INVERSE_CONST = PR-2; // inv = x^{PR-2}

const static size_t INT_PRECISION = BITS_LENGTH - FIXED_PRECISION; // 40 bits for integer part
const static TYPE MAX_POSITIVE = PR >>1;

const static TYPE ConstGapInTruncation = ((TYPE)1<<INT_PRECISION) - 1;
const static TYPE ConstEncode = (TYPE)1 << (BITS_LENGTH - 2);
const static TYPE ConstDecode = (TYPE)1 << (BITS_LENGTH - 2 - FIXED_PRECISION);

template<typename T> T additive_inverse(const T x);
template<typename T> T multiplicative_inverse(const T x);
template<typename T> bool test_inverse(T a, T b);
template<typename T> T add_mod(T a, T b);
template<typename T> T mult_mod(T a, T b);
template<typename T> T modPrime(T x);
template<typename T> void extend_gcd(const T, const T, T &x, T &y);
class gfpScalar;
template<typename T>
using eMatrix=Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
using gfpMatrix=Eigen::Matrix<gfpScalar, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
using gfpVector= Eigen::Matrix<gfpScalar, Eigen::Dynamic, 1>;


class gfpScalar
{
    // Operations between gfpScalars
    friend istream& operator>>(istream &is, gfpScalar &it);
    friend ostream& operator<<(ostream &os,const gfpScalar &it);
    friend bool operator<(const gfpScalar &a, const gfpScalar &b);
    friend bool operator==(const gfpScalar &a, const gfpScalar &b);
    friend bool operator!=(const gfpScalar &a, const gfpScalar &b);
    friend bool operator<=(const gfpScalar &a, const gfpScalar &b);
    friend bool operator>(const gfpScalar &a, const gfpScalar &b);
    friend bool operator>=(const gfpScalar &a, const gfpScalar &b);
    friend gfpScalar operator+(const gfpScalar &a, const gfpScalar &b);
    friend gfpScalar operator-(const gfpScalar &a, const gfpScalar &b);
    friend gfpScalar operator*(const gfpScalar &a, const gfpScalar &b);
    friend gfpScalar operator/(const gfpScalar &a, const gfpScalar &b);
    friend gfpScalar operator&(const gfpScalar &a,const gfpScalar &b);
    friend gfpScalar operator<<(const gfpScalar &a, const gfpScalar &x); // left shift bits
    friend gfpScalar operator>>(const gfpScalar &a, const gfpScalar &x);

    friend gfpScalar sqrt(const gfpScalar &x);
    friend gfpScalar rsqrt(const gfpScalar &x);
    friend gfpScalar square(const gfpScalar &x);
    template<typename T>
    friend gfpScalar pow(const gfpScalar &x,T exp);
    friend gfpScalar inverse(const gfpScalar &x);
    friend gfpScalar truncate(const gfpScalar &x, size_t d);

protected:
    TYPE value;
public:
    gfpScalar()=default;
    gfpScalar(TYPE _value):value(_value){}
    TYPE get_value()const{return value;}
    void set_value(TYPE &_v){value=_v;}

    gfpScalar operator-()const;
    gfpScalar& operator+=(const gfpScalar &other);
    gfpScalar& operator-=(const gfpScalar &other);
    gfpScalar& operator*=(const gfpScalar &other);
    gfpScalar& operator/=(const gfpScalar &other);
    gfpScalar& operator&=(const gfpScalar &other); // bitwise &
    gfpScalar& operator<<=(const gfpScalar &other); // left shitf
    gfpScalar& operator>>=(const gfpScalar &other); // right shift
    

    // Basic Math Methods
    gfpScalar& square(); // a^2
    gfpScalar& sqrt(); // sqrt(a)
    gfpScalar& inverse(); // 1/a
    gfpScalar& rsqrt(); // 1/sqrt(a)
    template<typename T>
    gfpScalar& pow(T exp);

    // Get Bit

    // Random Methods
    void random();
    void random_bit();

    // Pack/Unpack Methods into/from the octetStream
    void pack(octetStream &o);
    void unpack(octetStream &o);

    // For sint and sfix
    bool is_zero()const {return value == 0;}
    bool is_negative()const {return value > MAX_POSITIVE;}
    gfpScalar& truncate(size_t d = FIXED_PRECISION); // Right shift bits with sign bit.
    gfpScalar& magnify(); // Left shift bits with sign bit
    gfpScalar msb()const{return (TYPE)is_negative();}

};


#if defined(__x86_64__) && defined(__BMI2__)
inline uint64_t mul64(uint64_t a, uint64_t b, uint64_t *c)
{
    return _mulx_u64((unsigned long long)a, (unsigned long long)b, (unsigned long long*)c);
}
#else
inline uint64_t mul64(uint64_t a, uint64_t b, uint64_t *c)
{
    __uint128_t aa = a;
    __uint128_t bb = b;
    auto cc = aa*bb;
    *c = cc>>64;
    return (uint64_t)cc;
}
#endif

// #if  defined(__BMI2__)
// inline unsigned int mul32(unsigned int a, unsigned int b, unsigned int *c)
// {
//     return _mulx_u32((unsigned int)a, (unsigned int)b, (unsigned int*)c);
// }
// #else
inline unsigned int mul32(unsigned int a, unsigned int b, unsigned int *c)
{
    uint64_t aa = a;
    uint64_t bb = b;
    auto cc = aa*bb;
    *c = cc>>32;
    return (unsigned int)cc;
}
// #endif


template<typename T>
inline T add_mod(T a, T b)
{
    T ret = a+b;
    return (ret >= PR) ? (ret - PR) : ret;
}


template<typename T>
inline T mult_mod(T a, T b)
{
    T c = 0;
    T e, ret;
#if defined(PR_31) 
    e = mul32((unsigned int)a, (unsigned int)b, (unsigned int*)&c);
    ret = (e & PR) + ( (e>>MERSENNE_PRIME_EXP) ^ ((T)c<< (32-MERSENNE_PRIME_EXP)));
#elif defined(PR_61)
    e = mul64((uint64_t)a, (uint64_t)b, (uint64_t*)&c);
    ret = (e & PR) + ( (e>>MERSENNE_PRIME_EXP) ^ (c<< (64-MERSENNE_PRIME_EXP)));
#endif
    // c is most significant bits
    // e is low significant bits
    return (ret >= PR) ? (ret-PR) : ret;
}

template<typename T>
inline T modPrime(T x)
{
    T i = (x & PR) + (x >> MERSENNE_PRIME_EXP);
    return (i>=PR) ? i - PR: i;
}

template<typename T>
inline void extend_gcd(const T a, const T b, T& x, T& y)
{
    if(b==0){
        x = 1;
        y = 0;
        return;
    }
    extend_gcd(b, a%b, y, x);
    // a/b * x  execeed TYPE
    T tmp = mult_mod((T)(a/b), x);
    y = add_mod(y, additive_inverse(tmp));
    return;
}

template<typename T>
inline T additive_inverse(const T x)
{
    if(x==0){return 0;}// BUG LOG
    return PR^x;
}

template<typename T>
inline T multiplicative_inverse(const T it)
{
    T x, y;
    extend_gcd((const T)PR, (const T)it, x, y);
    return y;
}

template<typename T>
inline bool test_inverse(T a, T b)
{
    return mult_mod(a, b)==1;
}

inline istream& operator>>(istream &is, gfpScalar &it)
{
    is>>it.value;
    return is;
}

inline ostream& operator<<(ostream &os, const gfpScalar &it)
{
    os<<it.value;
    return os;
}

inline gfpScalar operator+(const gfpScalar &a, const gfpScalar &b)
{
    return gfpScalar(add_mod(a.value, b.value));
}

inline gfpScalar gfpScalar::operator-()const
{
    return gfpScalar(additive_inverse(value));
}

inline gfpScalar& gfpScalar::operator+=(const gfpScalar &other)
{
    value = add_mod(value, other.value);
    return *this;
}

inline gfpScalar operator-(const gfpScalar &a, const gfpScalar &b)
{
    return gfpScalar(add_mod(a.value, additive_inverse(b.value)));
}

inline gfpScalar& gfpScalar::operator-=(const gfpScalar &other)
{
    value = add_mod(value, additive_inverse(other.value));
    return *this;
}

inline gfpScalar operator*(const gfpScalar &a, const gfpScalar &b)
{
    return gfpScalar(mult_mod(a.value, b.value));
}

inline gfpScalar& gfpScalar::operator*=(const gfpScalar &other)
{
    value = mult_mod(value, other.value);
    return *this;
}

inline gfpScalar operator/(const gfpScalar &a, const gfpScalar &b)
{
    return gfpScalar(mult_mod(a.value, multiplicative_inverse(b.value)));
}

inline gfpScalar& gfpScalar::operator/=(const gfpScalar &other)
{
    value = mult_mod(value, multiplicative_inverse(other.value));
    return *this;
}

inline bool operator==(const gfpScalar &a, const gfpScalar &b)
{
    return a.value == b.value;
}

inline bool operator!=(const gfpScalar &a, const gfpScalar &b)
{
    return a.value != b.value;
}

inline bool operator<(const gfpScalar &a, const gfpScalar &b)
{
    return a.value < b.value;
}

inline bool operator<=(const gfpScalar &a, const gfpScalar &b)
{
    return (a.value < b.value)||(a.value == b.value);
}

inline bool operator>(const gfpScalar &a, const gfpScalar &b)
{
    return (a.value > b.value);
}

inline bool operator>=(const gfpScalar &a, const gfpScalar &b)
{
    return !(a.value < b.value);
}

/*************************************************
 * 
 *       Basic Math Methods
 * 
 * ***********************************************/
inline gfpScalar& gfpScalar::square()
{
    value = mult_mod(value, value);
    return *this;
}

inline gfpScalar square(const gfpScalar &x)
{
    return gfpScalar(mult_mod(x.value, x.value));
}


template<typename T>
inline gfpScalar& gfpScalar::pow(T exp)
{
    gfpScalar res(1);
    gfpScalar v_pow = *this;
    while(exp){
        if(exp&1){res *= v_pow;}
        exp >>=1;
        v_pow.square();
    }
    value = res.value;
    return *this;
}

template<typename T>
inline gfpScalar pow(const gfpScalar &x, T exp)
{
    gfpScalar res(1);
    gfpScalar v_pow = x;
    while(exp){
        if(exp&1){res *= v_pow;}
        exp >>=1;
        v_pow.square();
    }
    return res;
}

inline gfpScalar& gfpScalar::sqrt()
{
    assert(value>0);
    pow((TYPE)SQRT_EXP);
    if(value>=1 && value<MID_PR){return *this;}
    else{
        value = additive_inverse(value);
        return *this;
    }
}

inline gfpScalar sqrt(const gfpScalar &x)
{
    assert(x.value>0);
    gfpScalar res = pow(x, (TYPE)SQRT_EXP);
    if(res.value>=1 && res.value<MID_PR){return res;}
    else{
        return -res;
    }
}

// BUG LOG: Need to invoke sqrt to ensure the rang is [1, (p-1)/2]
inline gfpScalar& gfpScalar::rsqrt()
{
    if(value==0){return *this;} // For simplicity.

    sqrt();
    inverse();

    // pow((TYPE)RSQRT_CONST); // TODO: We can(?) calculate rsqrt in one pow.
    return *this;
}


inline gfpScalar rsqrt(const gfpScalar &x)
{
    if(x.value==0){return x;} // For simplicity.

    gfpScalar res = x;
    res.sqrt();
    res.inverse();
    return res;
    // return pow(x, RSQRT_CONST);
}

inline gfpScalar& gfpScalar::inverse()
{
    value = multiplicative_inverse(value);
    return *this;
}

inline gfpScalar operator&(const gfpScalar &a,const gfpScalar &b)
{
    return gfpScalar(a.value & b.value);
}

inline gfpScalar& gfpScalar::operator&=(const gfpScalar &other)
{
    value &= other.value;
    return *this;
}

inline gfpScalar operator<<(const gfpScalar &a, const gfpScalar &x)
{
    return gfpScalar(a.value<<x.value);
}

inline gfpScalar operator>>(const gfpScalar &a, const gfpScalar &x)
{
    return gfpScalar(a.value>>x.value);
}

inline gfpScalar& gfpScalar::operator<<=(const gfpScalar &other)
{
    value <<=other.value;
    return *this;
}

inline gfpScalar& gfpScalar::operator>>=(const gfpScalar &other)
{
    value >>=other.value;
    return *this;
}
/*************************************************
 * 
 *       Custom Methods

 * ***********************************************/
inline void gfpScalar::random()
{
#if defined(PR_31) 
    value = modPrime((unsigned int)secure_prng.get_uint());
#elif defined(PR_61)
    value = modPrime(secure_prng.get_word());
#endif
}

inline void gfpScalar::random_bit()
{
    value = secure_prng.get_bit();
}

inline void gfpScalar::pack(octetStream &o)
{
    o.append((octet*)&value, sizeof(value));
}

inline void gfpScalar::unpack(octetStream &o)
{
    o.consume((octet*)&value, sizeof(value));
}

// Truncate d bits if there is a sign bit.
inline gfpScalar& gfpScalar::truncate(size_t d)
{
    if(is_negative()){
        value = additive_inverse(value);
        value >>= d;
        value = additive_inverse(value);
    }else{
        value >>= d;
    }
    return *this;
}

inline gfpScalar truncate(const gfpScalar &x, size_t d = FIXED_PRECISION)
{
    gfpScalar res(x);
    return res.truncate(d);
}

inline gfpScalar& gfpScalar::magnify()
{
    if(is_negative()){
        value = additive_inverse(value);
        value <<= FIXED_PRECISION;
        value = additive_inverse(value);
    }else{
        value <<= FIXED_PRECISION;
    }
    return *this;
}

inline gfpScalar map_int_to_gfp(const STYPE &x)
{
    if(x<0) { return gfpScalar(x + (STYPE)PR);}
    return gfpScalar((TYPE)x);
}

inline STYPE map_gfp_to_int(const gfpScalar &x)
{
    STYPE res = x.get_value();
    if(res > MAX_POSITIVE){ res -= (STYPE)PR;}
    return res;
}


const static TYPE FACTOR = 1<<FIXED_PRECISION;

inline gfpScalar map_float_to_gfp(const double &x)
{
    return map_int_to_gfp((STYPE)floor(x * FACTOR));
}

inline double map_gfp_to_float(const gfpScalar &x)
{
    return ((double)map_gfp_to_int(x)) / (float)FACTOR;
}



} // namespace gfp_base

namespace Eigen{
template<> struct NumTraits<hmmpc::gfpScalar>
: NumTraits<unsigned int>
{
    typedef hmmpc::gfpScalar Real;
    typedef hmmpc::gfpScalar NonInteger;
    typedef hmmpc::gfpScalar Nested;

    enum {
    IsComplex = 0,
    IsInteger = 1,
    IsSigned = 1,
    RequireInitialization = 0,
    ReadCost = 1,
    AddCost = 3,
    MulCost = 3
  };
};
    template<typename BinaryOp>
    struct ScalarBinaryOpTraits<hmmpc::gfpScalar, hmmpc::TYPE, BinaryOp> { typedef hmmpc::gfpScalar ReturnType;  };
    template<typename BinaryOp>
    struct ScalarBinaryOpTraits<hmmpc::TYPE, hmmpc::gfpScalar, BinaryOp> { typedef hmmpc::gfpScalar ReturnType;  };

    template<typename BinaryOp>
    struct ScalarBinaryOpTraits<hmmpc::gfpScalar, int, BinaryOp> { typedef hmmpc::gfpScalar ReturnType;  };
    template<typename BinaryOp>
    struct ScalarBinaryOpTraits<int, hmmpc::gfpScalar, BinaryOp> { typedef hmmpc::gfpScalar ReturnType;  };

    template<typename BinaryOp>
    struct ScalarBinaryOpTraits<hmmpc::gfpScalar, int64_t, BinaryOp> { typedef hmmpc::gfpScalar ReturnType;  };
    template<typename BinaryOp>
    struct ScalarBinaryOpTraits<int64_t, hmmpc::gfpScalar, BinaryOp> { typedef hmmpc::gfpScalar ReturnType;  };

}


#endif