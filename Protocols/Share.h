#ifndef PROTOCOLS_SHARE_H_
#define PROTOCOLS_SHARE_H_

#include <iostream>
#include "Networking/Player.h"
#include "Math/gfpScalar.h"
#include "Math/gfpMatrix.h"
#include "Protocols/PhaseConfig.h"

/**
 * Some brief introduction to the class.
 * 
 */
namespace hmmpc
{

// Classes about Share
class ShareBase; // Store the basic static attributes about the Share.
class Share; // Store one share
class DoubleShare; // Store double shares

class Bit;

// Initiate the protocol to set the basic parameters.
void initiate_protocol(int n, int t, ThreadPlayer *_P, string fn, int _Pking = 0);


class ShareBase
{
public:
    /** Metadata **/
    static int threshold;
    static int n_players;
    static ThreadPlayer *P;
    static int Pking;
    static ifstream in;
    static PhaseConfig *Phase;// Control handsoff between offline and online phase.

    static PRNG PRNG_agreed; 
    static int start_party_PRG(){return threshold+1;}// t+1
    static int n_party_PRG(){return n_players - threshold;}//t+1

    // Buffers for multi-thread communication
    static octetStreams send_buffers;
    static octetStreams receive_buffers;
    // Buffers for multi-thread communication (with help of PRG)
    static gfpMatrix shares_buffers_PRG;
    static octetStreams send_buffers_PRG;
    static octetStreams receive_buffers_PRG;

    // Some fixed vandermond matrix.
    static gfpMatrix vandermonde_t; // Van(n, t)
    static gfpMatrix vandermonde_2t; // Van(n, 2t)
    static gfpMatrix vandermonde_n_t; // Van(n, n-t)

    // Some fixed reconstruction vector of f(0) conditioned on Pking (default=0)
    static gfpVector reconstruction_vector_t; // reconstruction t-sharing
    static gfpVector reconstruction_vector_2t; // reconstruction 2t-sharing

    // For 2t-degree, the share that is missed from Pi is variable, which can make the communication is 0.
    // For t-degree, we stipulate that the missing share is from P0.
    static gfpMatrix reconstruction_with_secret_2t; // This is to reconstruct Pi's share from the secret and known 2t shares.
    static gfpMatrix reconstruction_with_secret_t; // This is to reconstruct P0's share and P{t+1-2t} shares from the secret and known t shares.

    // Bits setting
    static gfpVector bits_coeff; // [1, 2, ..., 2^60]
    static void init_bits_coeff();
    

    static void set_input_file(string fn);
    static void close_input_file();

    // Vandermonde
    static void init_vandermondes();
    static gfpMatrix get_vandermonde(const size_t &xSize,const size_t &ySize);

    // Reconstruction Vector
    // The reconstruction is done by collecting degree+1 sharings from the set C.
    // The default set C is P0, P1, ..., P_{degree}, starting from the P0, which can be decided before executing.
    static bool is_in_reconstruction_set(const int &degree);
    static void init_reconstruction_vectors();
    static gfpScalar get_reconstruction_factor(const gfpScalar &point,const int &player_i,const int &degree);
    static gfpScalar get_reconstruction_factor(const int &player_i, const int &degree);// Point = 0
    static gfpVector get_reconstruction_vector(const gfpScalar &point, const int &degree);
    static gfpVector get_reconstruction_vector(const int &degree);// Point = 0

    // Having the secret as f(0). Assume this is the share of P{-1}
    // P-1 P0 P1 ... Pt Pt+1 ... P2t (This is the case that except_player=P0)
    //  y  n  y      y   y        y  : From these 2t+1 shares to reconstruct the share of P0
    //  y  n  y  ... y   n        n  : From these t+1 shares to reconstruct the share of Pt+1...P2t
    static gfpScalar get_reconstruction_factor_with_secret(const int &except_player, const gfpScalar &point, const int &player_i,const int &degree);
    static gfpVector get_reconstruction_vector_with_secret(const int &except_player, const gfpScalar &point, const int &degree);
};

class Share:public ShareBase
{
    friend class sint;
protected:
    gfpScalar secret;
    int degree;

    /** Basic operations about Share and DoubleShare **/
    // Notice that we use 'share' to represent one specific share.
    // And we use 'sharing' to represent a set of shares. [x]_t = {x_1, x_2, ..., x_n} /
    void calculate_sharing(const gfpScalar &secret, const int &degree, gfpScalar &share, octetStreams &os);
    void calculate_secret(gfpScalar &secret, const int &degree, const gfpScalar &share, octetStreams &os);

    /** Basic Operation about sharing **/
    void distribute_sharing(); // Calculate and distribute the sharing
    void reconstruct_secret(); // Pking collect the sharing and reconstruct the secret
    void send_share(int player_no);
    void receive_share(int player_no);

    // With PRG
    void distrbute_sharing_PRG(const gfpScalar &_secret, gfpScalar &_share);
    void distribute_2t_sharing_PRG(const gfpScalar &_secret, gfpScalar &_share);
    void distribute_t_sharint_PRG(const gfpScalar &_secret, gfpScalar &_share);
    void get_sharing_PRG(int player_no, gfpScalar &_share);
    void get_2t_sharing_PRG(int player_no, gfpScalar &_share);
    void get_t_sharing_PRG(int player_no, gfpScalar &_share);


public:
    gfpScalar share;

    Share():degree(threshold){}
    Share(const gfpScalar &_secret):secret(_secret), degree(threshold){}
    Share(const gfpScalar &_secret, const gfpScalar &_share): secret(_secret), degree(threshold), share(_share){}
    
    // Get
    int get_degree()const{return degree;}
    void set_degree(const int &_degree){degree = _degree;}
    gfpScalar get_secret()const{return secret;}

    // Set
    void double_degree(){degree = degree<<1;}
    void set_secret(const gfpScalar &_secret){secret = _secret;}

    Share& reduce_degree();
    Share& truncate();
    Share& reduce_truncate();
    Share& reduce_truncate(size_t precision);
    
    // Input
    void input_from_file(int player_no);
    void input_from_party(int player_no);
    void input_from_pking();
    void input_from_random(int player_no);

    void input_from_party_PRG(int player_no);
    void input_from_random_PRG(int player_no);

    // Reveal
    void reveal_to_party(int player_no); 
    void reveal_to_pking();
    gfpScalar reveal();

    // Random
    Share& random();
};

class DoubleShare: public Share
{ 
public:
    gfpScalar aux_share;
    DoubleShare():Share(){}
    DoubleShare(const gfpScalar &_secret):Share(_secret){}

    void input_from_random(int player_no);
    void input_from_ath_random(int player_no, TYPE a);

    // With PRG
    void input_from_random_PRG(int player_no);

    // Random
    DoubleShare& reduced_random(); // Randomize to the reduced sharing ([r]_t, [r]_2t)
    DoubleShare& truncated_random(Share &r_msb); // Randomize to the truncated sharing ([r/2^d]_t, [r]_t, [r_msb]_t)
    DoubleShare& reduced_truncated_random(Share &r_msb); // Randomize to the reduced and truncated sharing ([r/2^d]_t, [r]_2t, [r_msb]_t)
    DoubleShare& reduced_truncated_random(size_t precision);

    // Reveal the aux_sharing-sharing
    gfpScalar reveal_aux(int degree);
};






}

#endif