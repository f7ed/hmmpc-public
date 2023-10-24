#pragma once

#include "Protocols/ShareBundle.h"
#include "Math/gfpMatrix.h"
#include "Math/gfpScalar.h"
namespace hmmpc
{
/**
 * @brief Beaver Triple: [a] [b] [c] are all of degree-t with c = ab for input [x]_t and [y]_t
 * [x] = u - [a]
 * [y] = v - [b]
 * [xy] = ([x]+[a] - [a]) * ([y]+[b] - [b])
 *      = (u - [a]) * (v - [b])
 **     = uv - u[b] - v[a] + [c]
 * 
 * * Beaver Triple satisfies the linearity.
 * [x'] = i[x] = i (u - [a])  = iu - i[a] 
 * [y'] = k[y] = k (v - [b])  = kv - k[b] 
 * [x'y'] = (iu - i[a]) * (kv - k[b])
 *        = ( (iu)(kv) ) - (iu)(k[b]) - (kv)(i[a]) + ik[ab]
 * *      let u' = iu, v' = kv, [a'] = i[a], [b'] = k[b], then [c'] = [a'][b'] = ik[c]
 *        = u'v' - u'[b'] - v'[a'] + [c']
 */
class BeaverTriple 
{
protected:
    ShareBundle a;
    ShareBundle b;
    ShareBundle c;// c = a * b
    gfpMatrix u;// [x] = u - [a]t
    gfpMatrix v;// [y] = v - [b]_t

    /**
     * @brief [x'] = i[x] = i(u - [a]) = iu - i[a]
     *      [x'y'] = (iu - i[a]) * (v - [b])
     *             = (iu)v - iu[b] - v(i[a]) +i[ab]
     *  * let u' = iu, [a'] = i[a], [c'] = [a'][b] = i[c]
     */
    void x_affine_times(gfpScalar i)
    {u = i * u.array(); a.shares = i * a.shares.array(); c.shares = i * c.shares.array();}
    void y_affine_times(gfpScalar k)
    {v = k * v.array(); b.shares = k * b.shares.array(); c.shares = k * c.shares.array();}

    /**
     * @brief [x'] = [x] + j = (u+j) - [a] 
     *      [x'y'] = ((u+j) - [a]) * (v - [b])
     *             = (u+j)*v - (u+j)[b] - v[a] + [ab]
     *  * let u' = u+j
     */
    void x_affine_plus(gfpScalar j)
    {u = u.array() + j;}
    void y_affine_plus(gfpScalar w)
    {v = v.array() + w;}

public:
    BeaverTriple(const size_t &xSize, const size_t &ySize)
    :a(xSize, ySize), b(xSize, ySize), c(xSize, ySize), u(xSize, ySize), v(xSize,ySize)
    {u.setConstant(0); v.setConstant(0);}

    void set_shares(const gfpMatrix &a_shares, const gfpMatrix &b_shares,const gfpMatrix &c_shares)
    {a.shares = a_shares; b.shares = b_shares; c.shares = c_shares;}
    gfpMatrix& a_share(){gfpMatrix& ref = a.shares; return ref;}
    const gfpMatrix& a_share()const{const gfpMatrix &ref = a.shares; return ref;}
    gfpMatrix& b_share(){gfpMatrix& ref = b.shares; return ref;}
    const gfpMatrix& b_share()const{const gfpMatrix &ref = b.shares; return ref;}
    gfpMatrix& c_share(){gfpMatrix& ref = c.shares; return ref;}
    const gfpMatrix& c_share()const{const gfpMatrix &ref = c.shares; return ref;}

    gfpMatrix& u_value(){gfpMatrix& ref = u; return ref;}
    const gfpMatrix& u_value()const{const gfpMatrix &ref = u; return ref;}
    gfpMatrix& v_value(){gfpMatrix& ref = v; return ref;}
    const gfpMatrix& v_value()const{const gfpMatrix &ref = v; return ref;}

    size_t rows()const{return a.rows();}
    size_t cols()const{return a.cols();}
    
    
    // Recommand that do the times operation first.
    BeaverTriple& x_times(gfpScalar i) { x_affine_times(i); return *this; }
    BeaverTriple& x_plus(gfpScalar j) {x_affine_plus(j); return *this;}
    BeaverTriple& y_times(gfpScalar k) { y_affine_times(k); return *this; }
    BeaverTriple& y_plus(gfpScalar w) {y_affine_plus(w); return *this;}


    // Compute [xy]_t locally by the Beaver Triple.
    ShareBundle mult()
    {   ShareBundle res(rows(), cols());
        res.shares = u.array() * v.array() - u.array() * b.shares.array() - v.array() * a.shares.array()
                    + c.shares.array();
        return res;
    }
};


}// namespace hmmpc