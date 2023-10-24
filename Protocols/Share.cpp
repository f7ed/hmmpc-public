#include "Protocols/Share.h"
#include "Protocols/RandomShare.h"
#include "Protocols/Bit.h"
namespace hmmpc
{

/************************************************************************
 * 
 *       Definition of static member and member functions about ShareBase
 * 
 * **********************************************************************/

// Definition of the static member
/** Metadata **/
int ShareBase::threshold;
int ShareBase::n_players;
int ShareBase::Pking = 0;
ThreadPlayer * ShareBase::P;
ifstream ShareBase::in;
PhaseConfig * ShareBase::Phase;
PRNG ShareBase::PRNG_agreed;

octetStreams ShareBase::send_buffers;
octetStreams ShareBase::receive_buffers;

octetStreams ShareBase::send_buffers_PRG;
octetStreams ShareBase::receive_buffers_PRG;
gfpMatrix ShareBase::shares_buffers_PRG;

// Some fixed vandermond matrix.
gfpMatrix ShareBase::vandermonde_t; // Van(n, t)
gfpMatrix ShareBase::vandermonde_2t; // Van(n, 2t)
gfpMatrix ShareBase::vandermonde_n_t; // Van(n, n-t)

// Some fixed reconstruction vector of f(0) conditioned on Pking (default=0)
gfpVector ShareBase::reconstruction_vector_t; // reconstruction t-sharing
gfpVector ShareBase::reconstruction_vector_2t; // reconstruction 2t-sharing


// The first idx is the player_id of missing share
gfpMatrix ShareBase::reconstruction_with_secret_2t;// This is to reconstruct Pi's share from the secret and known 2t shares.
gfpMatrix ShareBase::reconstruction_with_secret_t;// This is to reconstruct Pi's share and P{t+1-2t} shares from the secret and known t shares.

// Bits
gfpVector ShareBase::bits_coeff(BITS_LENGTH);

void initiate_protocol(int n, int t, ThreadPlayer *_P, string fn, int _Pking)
{
    ShareBase::n_players = n;
    ShareBase::threshold = t;
    ShareBase::P = _P;
    ShareBase::set_input_file(fn);
    ShareBase::Pking = _Pking;
    ShareBase::init_vandermondes();
    ShareBase::init_reconstruction_vectors();

    ShareBase::init_bits_coeff();

    ShareBase::send_buffers.reset(n);
    ShareBase::receive_buffers.reset(n);
}

void ShareBase::set_input_file(string fn)
{
    in.open(fn.c_str());
    if((in).fail()){
        stringstream ss;
        ss << "Error opening "<<fn<<"."<<endl;
        throw file_error(ss.str().c_str());
    }
}

void ShareBase::close_input_file()
{
    in.close();
}

void ShareBase::init_bits_coeff()
{
    bits_coeff(0) = 1;
    for(size_t i = 1; i < BITS_LENGTH; i++){
        bits_coeff(i) = bits_coeff(i-1) << 1;
    }
}

/*************************************************
 * 
 *       Member Methods about Vandermonde Matrix
 * 
 * ***********************************************/
void ShareBase::init_vandermondes()
{
    vandermonde_t = get_vandermonde(n_players, threshold);
    vandermonde_2t = get_vandermonde(n_players, threshold<<1);
    vandermonde_n_t = get_vandermonde(n_players, n_players - threshold);
}

/**
 * @brief Vandermonde matrix without the first column.
 * 1 1 ... 1 
 * 2 4 ... 2^y
 * 3 9 ... 3^y
 * n n^2 ... n^y
 * @param xSize Equal to n, number of parties.
 * @param ySize Equal to degree.
 * @return gfpEMatrix 
 */
gfpMatrix ShareBase::get_vandermonde(const size_t &xSize,const size_t &ySize)
{
    gfpMatrix matrix(xSize, ySize);
    for(size_t i = 0; i < matrix.rows(); i++){
        gfpScalar prod = 1;
        gfpScalar x = i+1;
        for(size_t j = 0; j < matrix.cols(); j++){
            prod = prod*x;
            matrix(i, j) = prod;
        }
    }
    // return matrix.mod();
    return matrix;
}

/***************************************************************************************
 * 
 *       Member Methods about Reconstruction Vector of the Lagrange Interpolation
 * 
 * *************************************************************************************/

void ShareBase::init_reconstruction_vectors()
{
    reconstruction_vector_t = get_reconstruction_vector(threshold);
    reconstruction_vector_2t = get_reconstruction_vector((threshold<<1));

    // Mapped ID = original id + 1
    // P0 P1 ... Pt Pt+1 ... P2t+1
    // P0 owns secret f(0)
    // Pi owns f(i)

    // Each row corresponds a case that Pi's share is missing. i = 1 ... 2t+1
    reconstruction_with_secret_2t.resize(n_players, (threshold<<1)|1);
    for(size_t i = 0; i < n_players; i++){
        reconstruction_with_secret_2t.row(i) = get_reconstruction_vector_with_secret(i+1, i+1, threshold<<1);
    }

    // Assume we have f(0) f(2) ... f(t+1): secret + t shares from P0 ... Pt+1 without share of P1
    // These rows are arranged as f(t+2) ... f(2t+1) f(1): reconstruct t+1 shares of Pt+2, Pt+3,... P2t+1, P1
    reconstruction_with_secret_t.resize(n_players-threshold, threshold+1); // (t+1, t+1)
    // First t+1 rows
    for(int i = 0, id=threshold+2; i < n_players-threshold-1; i++, id++){
        reconstruction_with_secret_t.row(i) = get_reconstruction_vector_with_secret(1, id, threshold);
    }
    reconstruction_with_secret_t.row(n_players-threshold-1) = get_reconstruction_vector_with_secret(1, 1, threshold);
}


// The reconstruction set C is the set containing P0(Pking), P1, ..., P_{degree}.
// Pking is decided in advance, so are the reconstruction set. 
bool ShareBase::is_in_reconstruction_set(const int &degree)
{
    return (P->get_relative(Pking)<=degree);
}

/**
 * @brief Get the reconstruction factor of the player_i for the specific point.
 * C is the set containing P0, P1, ..., P_{degree}. (starting from Pking)
 * P0 is the default Pking.
 * factor of the player_i = {for all j: j!=i}\Phi (j-point)/(j-i)
 * @param point Could be any point of the function.
 * @param player_i 
 * @param degree degree of the polynomial
 * @return gfpScalar 
 */
gfpScalar ShareBase::get_reconstruction_factor(const gfpScalar &point,const int &player_i,const int &degree)
{
    gfpScalar factor(1);
    int n_relevant_player = degree+1;
    gfpScalar gfp_i(player_i+1); // Player ID shoud start from 1.
    for(int offset = 0; offset < n_relevant_player; offset++){
        int player_j = positive_modulo(Pking+offset, n_players);
        gfpScalar gfp_j(player_j+1);
        if(player_i!=player_j){
            factor *= (gfp_j - point)/(gfp_j - gfp_i);
        }
    }
    return factor;
}

// For the specific point = 0
gfpScalar ShareBase::get_reconstruction_factor(const int&player_i, const int &degree)
{
    return get_reconstruction_factor(gfpScalar(0), player_i, degree);
}

/**
 * @brief Get the Lagrange reconstruction vector for the specific point
 * C is the set containing P0, P1, ..., P_{degree}.
 * P0 is the default Pking.
 * reconstruction(i) = factor of the player_i = {for all j: j!=i}\Phi (j-point)/(j-i)
 * @param point 
 * @param degree 
 * @return gfpMatrix 
 */
gfpVector ShareBase::get_reconstruction_vector(const gfpScalar &point, const int &degree)
{
    int n_relevant_player = degree+1;
    gfpVector reconstruction(n_relevant_player,1);
    for(int i = 0; i < n_relevant_player; i++){
        int player_no = positive_modulo(Pking+i, n_players);
        reconstruction(i) = get_reconstruction_factor(point, player_no, degree);
    }
    return reconstruction;
}

// Reconstruction vector for the specific point = 0
gfpVector ShareBase::get_reconstruction_vector(const int &degree)
{
    return get_reconstruction_vector(gfpScalar(0), degree);
}

/**
 * @brief This is the case that except_player=P1
 * Having the secret as f(0). Assume this is the share of P{-1} --> P0
 *  P0 P1 P2 ... Pt+1        P2t+1 (mapped id)
 *  y  y  n      y   y        y  : From these 2t+1 shares to reconstruct the share of P1 (degree 2t)
 *  y  y  n  ... y   n        n  : From these t+1 shares to reconstruct the share of Pt+1...P2t (degree t)
 * @param except_player The share of this player that we need to calculate.
 * @param point 
 * @param player_i 
 * @param degree t or 2t
 * @return gfpScalar 
 */
gfpScalar ShareBase::get_reconstruction_factor_with_secret(const int &except_player, const gfpScalar &point, const int &player_i,const int &degree)
{
    gfpScalar factor(1);
    int n_relevant_player = degree+1;
    gfpScalar gfp_i(player_i);
    
    for(int offset = 0; offset < n_relevant_player+1; offset++){
        if(offset == except_player){continue;}
        gfpScalar gfp_j(offset);
        if(gfp_i != gfp_j){
            factor *= (gfp_j - point)/(gfp_j - gfp_i);
        }
    }
    return factor;
}

/**
 * @brief Get the Lagragnge reconstruction vector for the specific point.
 *  P0 P1 P2 ... Pt+1        P2t+1 (mapped id)
 * - degree = 2t: the factor is about P0 P1 ... P2t+2 except the 'except player'
 * - degree = t: the factor is about P0 ... Pt+1 except the 'except player'
 * 
 * @param except_player 
 * @param point 
 * @param degree 
 * @return gfpVector 
 */
gfpVector ShareBase::get_reconstruction_vector_with_secret(const int &except_player, const gfpScalar &point, const int &degree)
{
    int n_relevant_player = degree+1;
    gfpVector reconstruction(n_relevant_player);

    int idx = 0;
    for(int i = 0; i < n_relevant_player+1; i++){// P0 ... P2t+1
        if(i==except_player){continue;}
        reconstruction(idx) = get_reconstruction_factor_with_secret(except_player, point, i, degree);
        idx++;
    }
    return reconstruction;
}


/************************************************************************
 * 
 *       Definition of static member and member functions about Share
 * 
 * **********************************************************************/

/*************************************************
 * 
 *       Basic operation about the sharing
 * 
 * ***********************************************/

/**
 * @brief Calculate and pack the sharing of the secret of some degree.
 * We firstly use the Vandermonde matrix to calcualte the sharing of secret of some degree.
 * Then we store my own share into 'share', which will be stored into the specific instance later.
 * As for other parties shares, we pack them into the octetStreams os[n], each of which corresponds to one party.
 * 
 * @param secret [in]
 * @param degree [in]
 * @param share [out] My own share of the sharing.
 * @param os [out] Store other parties' shares
 */
void Share::calculate_sharing(const gfpScalar &secret, const int &degree, gfpScalar &share, octetStreams &os)
{
    gfpMatrix vandermonde = (degree==threshold)? vandermonde_t : vandermonde_2t;
    gfpVector random_coeffs(degree);
    random_matrix(random_coeffs);
    gfpVector sharing = (vandermonde * random_coeffs).array() + secret; 
    share = sharing(P->my_num());
    pack_row(sharing, os);
}

/**
 * @brief Calculate the secret from the sharing using the Lagrange Interpolation.
 * The reconstruction vector to point=0 has been evaluated in preprocessing stage.
 * 
 * @param secret [out] 
 * @param degree [in] degree of the sharing
 * @param share [in] P's share
 * @param os [in] The shares from parties of the reconstruction set.
 */
void Share::calculate_secret(gfpScalar &secret, const int &degree, const gfpScalar &share, octetStreams &os)
{
    gfpVector reconstruction = (degree==threshold)?reconstruction_vector_t:reconstruction_vector_2t;
    gfpVector sharing(degree+1);
    unpack_row(sharing, os);
    // If P is in the reconstruction set, the share of P is not in os because we avoid the loopback.
    if(is_in_reconstruction_set(degree)){
        sharing(P->my_num())=share;
    }
    secret = reconstruction.dot(sharing);
}

/**
 * @brief Calcuate and distribute the sharing of degree t of the secret.
 * 
 */
void Share::distribute_sharing()
{
    octetStreams os(n_players);
    calculate_sharing(secret, degree, share, os);
    P->send_respective(os);
}

/**
 * @brief Collect the sharing and reconstruct the secret.
 * 
 */
void Share::reconstruct_secret()
{
    octetStreams os(degree+1);
    // os[player_no] stores the share of P_{player_no}
    for(size_t player_no = 0; player_no < degree+1; player_no++)
    if(P->my_num()!= player_no){ // Avoiding the communication of loopback.
        P->receive_player(player_no, os[player_no]);
    }

    calculate_secret(secret, degree, share, os);
}

void Share::send_share(int player_no)
{
    octetStream o;
    share.pack(o);
    P->send_to(player_no, o);
}

void Share::receive_share(int player_no)
{
    octetStream o;
    P->receive_player(player_no, o);
    share.unpack(o);
}

// * Distribute and receive shares with help of PRG. 

void Share::distrbute_sharing_PRG(const gfpScalar &_secret, gfpScalar &_share)
{
    if(degree==threshold){ distribute_t_sharint_PRG(_secret, _share);}
    else{ distribute_2t_sharing_PRG(_secret, _share);}
}

/**
 * @brief Pi distribute a 2t-sharing.
 * Pi generates 2t shares from PRG. Pi calculate its own share from the 2t shares and its secret.
 * The communication is 0.
 *
 * Other parties can get its share directly from the PRG.
 */
void Share::distribute_2t_sharing_PRG(const gfpScalar &_secret, gfpScalar &_share)
{
    int _degree=threshold<<1;
    gfpVector shares_prng(_degree);
    random_matrix(shares_prng, PRNG_agreed);

    // For ease of coding, we assume n = 2t+1;
    gfpVector material(_degree+1);
    material(0) = _secret;
    material.tail(_degree) = shares_prng;
    _share = reconstruction_with_secret_2t.row(P->my_num()).dot(material);
}

/**
 * @brief Pi distribute a t-sharing.
 * Pi generates t shares from PRG. We stipulcate that the t shares are P1 ... Pt.
 * Pi calculates t+1 shares for Pt+1 ... P2t, and P0, from the t shares from PRG and its secret.
 * 
 * P1 ... Pt can get its share from PRG without communication.
 * Pt+1 ... P2t, and P0 need receive from Pi. (Assume Pi is not in them)
 * 
 */
void Share::distribute_t_sharint_PRG(const gfpScalar &_secret, gfpScalar &_share)
{
    assert(degree==threshold);
    // Mapped party
    // P0 P1 ... Pt Pt+1
    
    // shares from prng: P2 ... Pt+1
    gfpVector shares_prng(degree);
    random_matrix(shares_prng, PRNG_agreed);

    // material: P0 P2 ... Pt+1
    // True idx is P1 ... Pt
    gfpVector material(degree+1);
    material(0) = _secret;
    material.tail(degree) = shares_prng;
    
    // sharing(t+1, 1): Calculate shares for P_t+2 ... P_2t+1 P1
    // True idx is P_{t+1} ... P_{2t} P0
    gfpVector sharing = reconstruction_with_secret_t * material;

    if(P->my_num()>=1 && P->my_num()<=degree){// We can get shares from PRG directly.
        _share = shares_prng(P->my_num() - 1);
    }else if (P->my_num()==0){// Get from the sharing
        _share = sharing(degree);
    }else{ 
        _share = sharing(P->my_num() - degree - 1);
    }

    octetStreams os(degree+1);
    pack_row(sharing, os);
    P->send_respective(degree+1, degree+1, os);
}

void Share::get_sharing_PRG(int player_no, gfpScalar &_share)
{
    if(degree==threshold){get_t_sharing_PRG(player_no, _share);}
    else {get_2t_sharing_PRG(player_no, _share);}
}

// If my_num() != player_no, receive from player_no or get directly from PRG.
void Share::get_t_sharing_PRG(int player_no, gfpScalar &_share)
{
    gfpVector shares_prng(degree);
    random_matrix(shares_prng, PRNG_agreed);

    if(P->my_num()>=1 && P->my_num()<=degree){// We can get shares from PRG directly.
        _share = shares_prng(P->my_num() - 1);
    }else{ // Receive from player_no
        octetStream o;
        P->receive_player(player_no, o);
        _share.unpack(o);
    }
    return;
}

// Get from PRG directly.
void Share::get_2t_sharing_PRG(int player_no, gfpScalar &_share)
{
    gfpVector shares_prng(threshold<<1);
    random_matrix(shares_prng, PRNG_agreed);

    if(P->my_num()>player_no){
        _share = shares_prng(P->my_num()-1);
    }else{
        _share = shares_prng(P->my_num());
    }
}

/**
 * @brief The degree reduction protocol / multiplication protocol can reduce a sharing
 * of degree 2t to a sharing of degree t using DN's double randoms ([r]_t, [r]_2t).
 * 
 * @return Share& 
 */
Share& Share::reduce_degree()
{
    double_degree();
    assert(degree == threshold<<1);

    DoubleShare r;
    r.reduced_random(); // Get a reduced random sharing. ([r]_t, [r]_2t)
    share += r.aux_share;

    reveal();
    degree >>= 1;
    share = secret - r.share;

    // reveal_to_pking(); // Pking collects the shares of [x+r]_2t and reveal the secret.
    // degree >>=1;
    // input_from_pking(); // Pking distributes the shares of [x+r]_t
    // share -=r.share;
    return *this;
}

/**
 * @brief The truncation protocol can truncate d bits of a sharing of degree t. 
 * 
 * @return Share& 
 */
Share& Share::truncate()
{
    assert(degree == threshold);
    DoubleShare r;
    Share r_msb;
    r.truncated_random(r_msb);
    share += r.aux_share + ConstEncode;
    
    reveal();
    secret.truncate();

    gfpScalar is_overflow = (1-r_msb.share) * secret.msb();
    share = secret - r.share + ConstGapInTruncation * is_overflow - ConstDecode;
    return *this;
}

/**
 * @brief The reduction_and_truncation protocol can truncate d bits of a sharing of degree 2t.
 * 
 * @return Share& 
 */
Share& Share::reduce_truncate()
{
    double_degree();
    assert(degree == threshold<<1);
    DoubleShare r;
    Share r_msb;
    r.reduced_truncated_random(r_msb);

    share += r.aux_share + ConstEncode;// Encode to make sure MSB(a)=0
    
    reveal();
    degree >>=1;
    secret.truncate();

    gfpScalar is_overflow = (1-r_msb.share) * secret.msb();

    // Decode: bring another 1 bit error
    share = secret - r.share + ConstGapInTruncation * is_overflow - ConstDecode;

    
    return *this;
    
    // * A method to reduce the gap. (Just for test)
    // We can subtract 'p-(2^{l-d}-1)' or add '(2^{l-d}-1)'
    // We need to change the reduced_truncated random to the random triple.
    // gfpScalar c = secret.msb();
    // cout<<c<<endl;
    // share = secret - r.share + c * ( ((TYPE)1 << (BITS_LENGTH-FIXED_PRECISION) ) - 1);
    return *this;


    // *subtract
    // gfpScalar c = secret.msb();
    // cout<<c<<endl;
    // Share gap;
    // gap.share = 0;
    // cout<<gap.share<<endl;
    // for(int i = BITS_LENGTH - FIXED_PRECISION; i < BITS_LENGTH; i++){
    //     cout<<((TYPE)1<<i)<<endl;;
    //     gap.share = gap.share + ((TYPE)1<<i)*c;
    // }
    // cout<<gap.share<<endl;
    // share = secret - r.share -gap.share;
    // return *this;
}

// TODO: Encode to make Sure MSB(a) = 0
Share& Share::reduce_truncate(size_t precision)
{
    double_degree();
    assert(degree == threshold<<1);
    DoubleShare r;
    r.reduced_truncated_random(precision);
    share += r.aux_share;

    reveal();
    degree >> 1;
    secret.truncate(precision);
    share = secret - r.share;
    return *this;
}

/*********************************************************************
 * 
 *       Input Methods
 * 
 * There are serveral input methods with tiny difference.
 * The only difference is the source of the secret.
 * 
 * 1. input_from_file: secret from the file
 * 2. input_from_party: secret from the assigned value
 * 3. input_from_pking: the specific case of input_from_party
 * 4. input_from_random: secret from the PRNG
 * 
 * *******************************************************************/

void Share::input_from_file(int player_no)
{
    if(player_no == P->my_num()){
        in>>secret; // Input secret from the file.
        distribute_sharing();
    }else{receive_share(player_no);}
}

void Share::input_from_party(int player_no)
{
    if(player_no == P->my_num()) { distribute_sharing();}
    else { receive_share(player_no);}
}

void Share::input_from_pking()
{
    input_from_party(Pking);
}

void Share::input_from_random(int player_no)
{
    if(player_no == P->my_num()) {
        secret.random();
        distribute_sharing();
    }else {receive_share(player_no);}
}

void Share::input_from_party_PRG(int player_no)
{
    if(player_no == P->my_num()){distrbute_sharing_PRG(secret, share);}
    else{get_sharing_PRG(player_no, share);}
}

void Share::input_from_random_PRG(int player_no)
{
    if(player_no == P->my_num()){
        secret.random();// This PRG is different from the PRG used to generate shares.
        distrbute_sharing_PRG(secret, share);
    }else{get_sharing_PRG(player_no, share);}
}

/*********************************************************************************************
 * 
 *       Reveal Methods
 * 
 * There are serveral reveal methods with tiny difference.
 * The only difference is the range of the reveal.
 * 
 * 1. reveal_to_party(player_no): The specific party collects shares from the reconstuction set and calcuate the secret.
 * 2. reveal_to_Pking() : The specific party is Pking.
 * 3. reveal(): reveal_to_Ping at first, then Pking send the secret to other parties.
 * 
 * 
 * *******************************************************************************************/

/**
 * @brief Reveal the secret only to the P_{player_no}
 * 
 * @param player_no 
 */
void Share::reveal_to_party(int player_no)
{
    // If it is in the reconstruction set, send the share to Pking.
    // We exclude the P to avoid the loopback.
    if(P->my_num()!=player_no && is_in_reconstruction_set(degree)){
        send_share(player_no);
    }
    
    // Pking collects the sharing and reconstruct it.
    if(P->my_num()==player_no){
        reconstruct_secret();
    }   
}

// Reveal the secret only to Pking
void Share::reveal_to_pking()
{
    reveal_to_party(Pking);
}

// Reveal to Pking, then send the secret to other parties.
gfpScalar Share::reveal()
{
    reveal_to_pking();
    octetStream o;
    if(P->my_num()==Pking){// Pking send the secret to other parties
        secret.pack(o);
        P->send_all(o);
    }else{
        P->receive_player(Pking, o);
        secret.unpack(o);
    }
    return secret;
}

/*********************************************************************
 * 
 *       Random Methods
 * 
 * 
 * *******************************************************************/
Share& Share::random()
{
    share = 0;
    return *this;
    
    RandomShare::get_random(RandomShare::queueRandom, share);
    return *this;
}


/************************************************************************
 * 
 *       Definition of member functions about DoubleShare
 * 
 * **********************************************************************/

/**
 * @brief Input the secret from the PNRG to generate [a]_t and [a]_2t
 * It generates a pair of sharing of the same secret.
 * One is of degree t and the other is of degree 2t.
 * 
 * @param player_no 
 */
void DoubleShare::input_from_random(int player_no)
{
    assert(degree == threshold);
    if(player_no == P->my_num()){
        secret.random();
        octetStreams os(n_players);
        calculate_sharing(secret, degree, share, os);
        calculate_sharing(secret, degree<<1, aux_share, os);
        P->send_respective(os);
    }else{
        octetStream o;
        P->receive_player(player_no, o);
        share.unpack(o);
        aux_share.unpack(o);
    }
}

void DoubleShare::input_from_ath_random(int player_no, TYPE a)
{
    assert(degree == threshold);
    if(player_no == P->my_num()){
        secret.random();
        octetStreams os(n_players);
        calculate_sharing(secret, degree, share, os);
        calculate_sharing(secret.pow(a), degree, aux_share, os);
        P->send_respective(os);
    }
    else{
        octetStream o;
        P->receive_player(player_no, o);
        share.unpack(o);
        aux_share.unpack(o);
    }
}

void DoubleShare::input_from_random_PRG(int player_no)
{
    assert(degree==threshold);
    if(player_no == P->my_num()){
        secret.random();
        distribute_t_sharint_PRG(secret, share);
        distribute_2t_sharing_PRG(secret, aux_share);
    }else{
        get_t_sharing_PRG(player_no, share);
        get_2t_sharing_PRG(player_no, aux_share);
    }
}

/*********************************************************************
 * 
 *       Random Methods
 * 
 * 
 * *******************************************************************/
DoubleShare& DoubleShare::reduced_random()
{
#ifdef ZERO_OFFLINE
    share = 0;
    aux_share = 0;
    return *this;
#endif

    DoubleRandom::get_random_pair(DoubleRandom::queueReducedRandom, share, aux_share);// [r]_t, [r]_2t
    return *this;
}

DoubleShare& DoubleShare::truncated_random(Share &r_msb)
{
#ifdef ZERO_OFFLINE
    share = 0;
    aux_share = 0;
    r_msb.share = 0;
    return *this;
#endif

    // DoubleRandom::get_random_pair(DoubleRandom::queueTruncatedRandom, share, aux_share);//  [r/2^d]_t, [r]_2
    DoubleRandom::get_random_triple(DoubleRandom::queueTruncatedRandom, share, aux_share, r_msb.share);
    return *this;
}

DoubleShare& DoubleShare::reduced_truncated_random(Share &r_msb)
{
#ifdef ZERO_OFFLINE
    // share = 0;
    // aux_share = 0;
    // r_msb.share = 0;

    // Just for test the gap
    gfpScalar tmp = MAX_POSITIVE;
    share = tmp.truncate();
    aux_share = MAX_POSITIVE;
    r_msb.share = 0;
    return *this;
#endif

    // DoubleRandom::get_random_pair(DoubleRandom::queueReducedTruncatedRandom, share, aux_share); // [r/2^d]_2t, [r]_t
    DoubleRandom::get_random_triple(DoubleRandom::queueReducedTruncatedRandom, share, aux_share, r_msb.share);
    return *this;
}

DoubleShare& DoubleShare::reduced_truncated_random(size_t precision)
{
#ifdef ZERO_OFFLINE
    share = 0;
    aux_share = 0;
    return *this;
#endif

    DoubleRandom::get_random_pair(DoubleRandom::queueReducedTruncatedWithPrecisionRandom, share, aux_share); // [r/2^d]_2t, [r]_t
    return *this;
}

gfpScalar DoubleShare::reveal_aux(int degree)
{
    Share aux;
    aux.share = aux_share;
    aux.set_degree(degree);
    return aux.reveal();
}

}