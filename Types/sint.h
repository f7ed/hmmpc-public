#ifndef TYPES_SINT_H_
#define TYPES_SINT_H_

#include "Types/cint.h"
#include "Protocols/Share.h"
#include "Protocols/ShareBundle.h"

namespace hmmpc
{

class sint
{
    friend sint operator+(const sint &a, const sint &b);
    friend sint operator-(const sint &a, const sint &b);
    friend sint operator*(const sint &a, const sint &b);
    friend sint operator/(const sint &a, const sint &b);
    friend sint operator<(const sint &a, const sint &b);
    friend sint operator==(const sint&a, const sint &b);

    friend sint operator+(const sint &a, const cint&b);
    friend sint operator+(const cint &a, const sint&b);
    friend sint operator-(const sint &a, const cint&b);
    friend sint operator-(const cint &a, const sint&b);
    friend sint operator*(const sint &a, const cint&b);
    friend sint operator*(const cint &a, const sint&b);
    friend sint operator/(const sint &a, const cint&b);
    friend sint operator/(const cint &a, const sint&b);
protected:
    Share sharing;

public: 
    sint(){}
    sint(const STYPE &x){share() = map_int_to_gfp(x);}
    sint(const gfpScalar &_v){share() = _v;}

    gfpScalar& share(){gfpScalar &ref = sharing.share; return ref;}
    const gfpScalar& share()const{const gfpScalar &ref = sharing.share; return ref;}
    gfpScalar& secret(){gfpScalar &ref = sharing.secret; return ref;}
    const gfpScalar& secret()const{const gfpScalar &ref = sharing.secret; return ref;}

    // Input
    void input_from_file(int player_no);
    void input_from_party(int player_no); // This is only for developing.
    void input_from_random(int player_no);

    // Reveal
    void reveal_to_party(int player_no);
    cint reveal();

    sint get_LSB();
    sint get_MSB();
};


/*********************************************
 *       Basic Operations
 * *******************************************/

inline sint operator+(const sint&a, const sint&b){return sint(a.share()+b.share());}
inline sint operator-(const sint&a, const sint&b){return sint(a.share()-b.share());}

inline sint operator+(const sint&a, const cint &b){return sint(a.share() + b.value);}
inline sint operator+(const cint&a, const sint &b){return sint(a.value + b.share());}
inline sint operator-(const sint&a, const cint &b){return sint(a.share() - b.value);}
inline sint operator-(const cint&a, const sint &b){return sint(a.value - b.share());}
inline sint operator*(const sint&a, const cint &b){return sint(a.share() * b.value);}
inline sint operator*(const cint&a, const sint &b){return sint(a.value * b.share());}
inline sint operator/(const sint&a, const cint &b){return sint(a.share() / b.value);}
inline sint operator/(const cint&a, const sint &b){return sint(a.value / b.share());}

inline sint operator*(const sint&a, const sint&b)
{
    Share res;
    res.share = a.share() * b.share();
    res.reduce_degree();
    return sint(res.share);
}

/*********************************************
 *       Input Methods
 * *******************************************/
inline void sint::input_from_file(int player_no)
{
    if(ShareBase::P->my_num()==player_no){
        cint secret;
        ShareBase::in>>secret;
        sharing.set_secret(secret.value);
    }
    sharing.input_from_party(player_no);
}

inline void sint::input_from_party(int player_no)
{
    sharing.input_from_party(player_no);
}

inline void sint::input_from_random(int player_no)
{
    sharing.input_from_random(player_no);
}

/*********************************************
 *       Reveal Methods
 * *******************************************/
inline void sint::reveal_to_party(int player_no)
{
    sharing.reveal_to_party(player_no);
    if(ShareBase::P->my_num()==player_no){
        cout<<"Ouput: "<<map_gfp_to_int(secret())<<endl;
    }
}

inline cint sint::reveal()
{
    return cint(sharing.reveal());
}

}
#endif