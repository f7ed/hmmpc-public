#ifndef TYPES_SFIX_H_
#define TYPES_SFIX_H_

#include "Types/sint.h"
#include "Types/cfix.h"
#include "Protocols/Share.h"
namespace hmmpc
{

class sfix:public sint
{
    friend sfix operator+(const sfix &a, const sfix &b);
    friend sfix operator-(const sfix &a, const sfix &b);
    friend sfix operator*(const sfix &a, const sfix &b);
    friend sfix operator/(const sfix &a, const sfix &b);
    friend sfix mult(const sfix&a, const sfix&b, size_t precision);

    friend sfix operator+(const sfix &a, const cfix &b);
    friend sfix operator+(const cfix &a, const sfix &b);
    friend sfix operator-(const sfix &a, const cfix &b);
    friend sfix operator-(const cfix &a, const sfix &b);
    friend sfix operator*(const sfix &a, const cfix &b);
    friend sfix operator*(const cfix &a, const sfix &b);

    sfix(const gfpScalar &_v):sint(_v){}
public:
    sfix(){}
    sfix(const double &x){secret() = map_float_to_gfp(x);}

    // Input
    void input_from_file(int player_no);
    void input_from_party(int player_no);

    // Reveal
    void reveal_to_party(int player_no);
    cfix reveal();

    void reduce_truncate(){sharing.reduce_truncate();}
    void reduce_truncate(size_t precision){sharing.reduce_truncate(precision);}
};

// void test_fix(const double &obj, const sfix &res)
// {
//     cout<<"expected "<<obj<<", got "<<res.reveal()<<endl;
// }

inline sfix operator+(const sfix&a, const sfix &b) {return sfix(a.share() + b.share());}
inline sfix operator-(const sfix&a, const sfix &b) {return sfix(a.share() - b.share());}

inline sfix operator+(const sfix&a, const cfix &b) {return sfix(a.share() + b.value);}
inline sfix operator+(const cfix&a, const sfix &b) {return sfix(a.value + b.share());}
inline sfix operator-(const sfix&a, const cfix &b) {return sfix(a.share() * b.value);}
inline sfix operator-(const cfix&a, const sfix &b) {return sfix(a.value * b.share());}

inline sfix operator*(const sfix&a, const sfix &b)
{
    Share res;
    res.share = a.share() * b.share();
    res.reduce_truncate();
    return sfix(res.share);
}

inline sfix operator*(const sfix&a, const cfix &b)
{
    Share res;
    res.share = a.share() * b.value;
    res.truncate();
    return sfix(res.share);
}

inline sfix operator*(const cfix&a, const sfix&b) {return b*a;}

inline sfix mult(const sfix&a, const sfix&b, size_t precision)
{
    sfix res;
    res.share() = a.share() * b.share();
    res.reduce_truncate(precision);
    return res;
}

/*********************************************
 *       Input Methods
 * *******************************************/
inline void sfix::input_from_party(int player_no)
{
    sharing.input_from_party(player_no);
}

inline void sfix::input_from_file(int player_no)
{
    if(ShareBase::P->my_num()==player_no){
        cfix secret;
        ShareBase::in>>secret;
        sharing.set_secret(secret.value);
    }
    sharing.input_from_party(player_no);
}

/*********************************************
 *       Reveal Methods
 * *******************************************/
inline void sfix::reveal_to_party(int player_no)
{
    sharing.reveal_to_party(player_no);
    if(ShareBase::P->my_num()==player_no){
        cout<<"Ouput: "<<map_gfp_to_float(secret())<<endl;
    }
}

inline cfix sfix::reveal()
{
    return cfix(sharing.reveal());
}
}

#endif
