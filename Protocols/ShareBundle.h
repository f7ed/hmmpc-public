#ifndef PROTOCOLS_SHAREVECTOR_H_
#define PROTOCOLS_SHAREVECTOR_H_

#include "Protocols/Share.h"
#include "Protocols/PhaseConfig.h"

namespace hmmpc
{
class BitBundle;
class BeaverTriple;

class ShareBundle:public ShareBase
{
    friend class sintMatrix;
protected:
    gfpMatrix secrets;
    int degree;

    // Matrix-grained Operations to sharings
    void calculate_sharings(const gfpMatrix &secrets, const int &degree, gfpMatrix &shares, octetStreams &os);
    void calculate_secrets(gfpMatrix &secrets, const int &degree, const gfpMatrix &shares, octetStreams &os);
    
    void distribute_sharings();
    void reconstruct_secrets();
    void send_shares(int player_no);
    void receive_shares(int player_no);
    void send_secrets();// send to all other parties.
    void receive_secrets(int player_no);// recv from


    // Row-grained Operations to sharings
    void calculate_block_sharings(const int &degree, const int &start_row, const int &n_rows, octetStreams &os);
    void calculate_block_secrets(const int &degree, const int &start_row, const int &n_rows, octetStreams &os);

    // * With PRG
    void calculate_2t_sharings_PRG(const gfpMatrix&_secrets, gfpMatrix &_shares);
    void calculate_t_sharings_PRG(const gfpMatrix&_secrets, gfpMatrix &_shares, octetStreams &os);
    void get_2t_sharings_PRG(int player_no, gfpMatrix &_shares);
    void get_t_sharings_PRG(int player_no, gfpMatrix &_shares);

    void distribute_sharings_PRG(const gfpMatrix &_secrets, gfpMatrix &_shares);
    void get_sharings_PRG(int player_no, gfpMatrix &_share);

    void get_t_sharings_PRG_request(int player_no, gfpMatrix &shares_prng, octetStream &o_receive);
    void get_t_sharings_PRG_wait(int player_no, const gfpMatrix &shares_prng, gfpMatrix &_shares, octetStream &o_receive);

    void calculate_block_t_sharings_PRG(const int &start_row, const int &n_rows, octetStreams &os_send);
    void get_block_t_sharings_PRG_request(int player_no, const int &start_row, const int &n_rows, gfpMatrix &shares_prng, octetStream &o_receive);
    void get_block_t_sharings_PRG_wait(int player_no, const int &start_row, const int &n_rows, const gfpMatrix &shares_prng, octetStream &o_receive);
    
    // Complicated functions: Maxpool
    void seqMaxpoolRowwise(ShareBundle &maxRes, ShareBundle &maxIdx)const;
    ShareBundle hierMaxpoolRowwise(int depth, size_t num, const ShareBundle &origin, ShareBundle &maxPrime)const;
    ShareBundle hierMaxpoolRowwise(int depth, size_t num, const ShareBundle &origin)const; // without maxpoolPrime

    // Use the two-layer multiplication in one round technique.
    ShareBundle hierMaxpoolRowwise_opt(int depth, size_t num, const ShareBundle &origin, ShareBundle &maxPrime)const;
    ShareBundle hierMaxpoolRowwise_opt(int depth, size_t num, const ShareBundle &origin)const; // without maxpoolPrime
    
public:
    gfpMatrix shares;
    ShareBundle():degree(threshold){}
    // BUG LOG: Need to setConsts(0). resize() dose not initalize to 0.
    ShareBundle(const size_t &len):ShareBundle(){resize(len,1); initializeZero();}
    ShareBundle(const size_t &xSize, const size_t &ySize):ShareBundle(){resize(xSize, ySize); initializeZero();}

    void initializeZero(){secrets.setConstant(0); shares.setConstant(0);}
    void resize(const size_t &xSize, const size_t &ySize)
    {secrets.resize(xSize, ySize); shares.resize(xSize, ySize); }

    gfpMatrix& secret(){gfpMatrix& ref = secrets; return ref;}
    const gfpMatrix& secret()const{const gfpMatrix &ref = secrets; return ref;}

    // Get
    int get_degree()const{return degree;}
    gfpMatrix get_secrets()const{return secrets;}
    size_t size()const{return shares.size();}
    size_t rows()const{return shares.rows();}
    size_t cols()const{return shares.cols();}

    // Set in matrix-grained
    void double_degree(){degree = degree<<1;}
    void set_degree(const int &_d){degree = _d;}
    void set_secret(const size_t &idx, const gfpScalar &_secret){secrets(idx) = _secret;}
    void set_secret(const gfpScalar &_secret){secrets.setConstant(_secret);}
    void set_secrets(const gfpMatrix &_secrets){secrets = _secrets;}

    // Set in row-grained
    void set_row(const size_t &row_no, const ShareBundle&other)
    {shares.row(row_no)<<other.shares;}
    
    // * Main Operations 
    ShareBundle& reduce_degree(); // reduce [x]_2t -> [x]_t
    ShareBundle& truncate(); // truncate [x]_t -> [x/2^d]_t
    ShareBundle& truncate(size_t precision); // truncate [x]_t -> [x/2^p]_t
    ShareBundle& reduce_truncate(); // reduce [x]_2t -> [x/2^d]_t
    ShareBundle& reduce_truncate(size_t logLearningRate, size_t logMiniBatch);
    ShareBundle& reduce_truncate(size_t precision);//reduce [x]_2t -> [x/2^p]_t
    ShareBundle& reduce_truncate(vector<size_t> &precision);

    // Two layer multiplication: compute [xy] from input [x]_2t and [y] (input wire)
    ShareBundle& reduce_degree_1stLayer(const ShareBundle &y, BeaverTriple &triple);
    // Generalize two layer-mult: compute ([xy1], [xy2], ...) from input [x]_2t and ([y1], [y2], ...)
    ShareBundle& reduce_degree_1stLayer(const vector<ShareBundle>&y, vector<BeaverTriple>&triples);

    // * Input methods
    // The normal version.
    void input_from_file(int player_no);
    void input_from_party(int player_no);
    void input_from_pking();
    void input_from_random(int player_no);

    // The multi-thread version.
    // The request part of input methods in multi-thread version.
    void input_from_file_request(int player_no, octetStreams &os_send, octetStream &o_receive);
    void input_from_party_request(int player_no, octetStreams &os_send, octetStream &o_receive);
    void input_from_random_request(int player_no, octetStreams &os_send, octetStream &o_receive);
    // The wait part of input methods in multi-thread version.
    void finish_input_from(int player_no, octetStreams &os_send, octetStream &o_receive);

    // Use the static buffers to input.
    void input_from_file_request(int player_no);
    void input_from_party_request(int player_no);
    void input_from_pking_request();
    void input_from_random_request(int player_no);
    void finish_input_from(int player_no);
    void finish_input_from_pking();

    // * With PRG (Assme n = 2t+1)
    // Distributing a 2t-sharing is easy without communication.
    // Distributing a t-sharing requires communication from Pi to {Pt+1, ..., P2t, P0}.
    int start_party_PRG()const{return ShareBase::threshold+1;}// t+1
    int n_party_PRG()const{return ShareBase::n_players - ShareBase::threshold;}//t+1
    // Normal version
    void input_from_party_PRG(int player_no);
    void input_from_random_PRG(int player_no);

    // Only for t-sharing since 2t-sharing can be shared without communication
    // (You can use the normal version to input a 2t-sharing) 
    void input_from_party_request_PRG(int player_no, octetStreams &os_send, gfpMatrix &shares_prng, octetStream &o_receive);
    void input_from_random_request_PRG(int player_no, octetStreams &os_send, gfpMatrix &shares_prng, octetStream &o_receive);
    void finish_input_from_PRG(int player_no, octetStreams &os_send, const gfpMatrix &shares_prng, octetStream &o_receive);

    // Use the static buffers to input.
    void input_from_party_request_PRG(int player_no);
    void input_from_random_request_PRG(int player_no);
    void finish_input_from_PRG(int player_no);

    // Reveal
    void reveal_to_party(int player_no);
    void reveal_to_pking();
    gfpMatrix reveal();
    gfpMatrix reveal_truncate(size_t precision);
    gfpMatrix reveal_truncate(vector<size_t> &precision);

    // * Partition the matrix on row-grained and disperse the operations to every party rather only Pking.
    void partition_rows(size_t &startRow, size_t &nRows); // The rule of partition on matrix.

    // Dispersed version of reveal_to_Pking.
    void reveal_blocks_dispersed();
    void reveal_block_from_party_request(int player_no, octetStreams &os_send, octetStream &o_receive);
    void finish_reveal_block_from_party(int player_no, octetStreams &os_send, octetStream &o_receive);

    // Dispersed version of input_from_Pking.
    void input_blocks_dispersed();
    void input_block_from_party_request(int player_no, const int &startRow, const int &nRows, octetStreams &os_send, octetStream &o_receive);
    void finish_input_block_from_party(int player_no, const int &startRow, const int &nRows, octetStreams &os_send, octetStream &o_receive);
    // Dispersed version of input_from_Pking. (With help of PRG)
    void input_blocks_dispersed_PRG();
    void input_block_from_party_request_PRG(int player_no, const int &startRow, const int &nRows, octetStreams &os_send, gfpMatrix &shares_prng, octetStream &o_receive);
    void finish_input_block_from_party_PRG(int player_no, const int &startRow, const int &nRows, octetStreams &os_send, const gfpMatrix &shares_prng, octetStream &o_receive);


    // Dispersed version of reveal
    gfpMatrix reveal_dispersed();
    gfpMatrix reveal_truncate_dispersed(size_t precision);// reconstruct the secret and truncate
    gfpMatrix reveal_truncate_dispersed(vector<size_t> &precision);

    // Random
    ShareBundle& random();

    // *Complicated Operations
    ShareBundle unbounded_mult();
    ShareBundle unbounded_prefix_mult();
    ShareBundle unbounded_postfix_mult();
    ShareBundle evalFunc(string fn, size_t degree);// Evaluate the function result of each entry given input X and degree.

    // *
    BitBundle get_LSB()const; // 2 layer circuit to compute LSB in 2 rounds.
    BitBundle get_MSB()const;

    ShareBundle ReLU()const;
    BitBundle deltaReLU()const;
    void ReLU(ShareBundle &deltaReLU, ShareBundle &relu)const; // 3 rounds to compute ReLU

    ShareBundle get_LSB_impared()const;// Remove the last layer of multiplication in LSB circuit.
    
    // Calculate along with the Beaver triples for the second layer.
    // BUG LOG: Note that the derivative class can be cast to the base class but the base class cannot be cast to the derivative class
    BitBundle get_LSB_opt(vector<ShareBundle> &y, vector<BeaverTriple> &triples)const;
    BitBundle get_MSB_opt(vector<ShareBundle> &y, vector<BeaverTriple> &triples)const;
    BitBundle deltaReLU_opt(vector<ShareBundle> &y, vector<BeaverTriple> &triples)const;
    void ReLU_opt(ShareBundle &deltaReLU, ShareBundle &relu)const;
    ShareBundle ReLU_opt()const;
    

    void MaxPrimeRowwise(ShareBundle &maxIdx)const;
    void MaxRowwise(ShareBundle &maxRes)const;
    void MaxRowwise(ShareBundle &maxRes, ShareBundle &maxIdx)const;

    void MaxRowwise_opt(ShareBundle &maxRes)const;
    void MaxRowwise_opt(ShareBundle &maxRes, ShareBundle &maxIdx)const;

    gfpMatrix bound_power()const;// Bound power on each entry x to get alpha where 2^alpha <= x < 2^alpha+1 
    gfpMatrix bound_power_paralle()const;
    gfpMatrix bound_power_with_bits()const;

    // Just for Tests
    void ReLU_opt_test(ShareBundle &deltaReLU, ShareBundle &relu)const; // Use the two-layer mult  techniqueto compute ReLU in 2 rounds
};

class DoubleShareBundle:public ShareBundle
{
public:
    gfpMatrix aux_shares;
    DoubleShareBundle():ShareBundle(){}
    DoubleShareBundle(const size_t &len):DoubleShareBundle(){resize(len, 1);}
    DoubleShareBundle(const size_t &xSize, const size_t &ySize):DoubleShareBundle(){resize(xSize, ySize);}
    void resize(const size_t &xSize, const size_t ySize){secrets.resize(xSize, ySize); shares.resize(xSize, ySize); aux_shares.resize(xSize, ySize);}

    // Set in row-grained
    void set_row(const size_t &row_no, const DoubleShareBundle&other)
    {shares.row(row_no)<<other.shares; aux_shares.row(row_no)<<other.aux_shares;}

    void input_from_random(int player_no);

    // Input in parallel.
    void input_from_random_request(int player_no, octetStreams &os_send, octetStream &o_receive);
    void finish_input_from(int player_no, octetStreams &os_send, octetStream &o_receive);

    // * With PRG
    void input_from_random_PRG(int player_no);

    // Multi-thread Version
    // 1. Random the secrets and request to send the t-sharings.
    // void input_from_random_request_PRG(int player_no, octetStreams &os_send, gfpMatrix &shares_prng, octetStream &o_receive);
    // 2. Calculate the 2t-sharings without communication.
    void input_aux_from_random_PRG(int player_no);
    // 3. Wait to receive the t-sharings.
    // void finish_input_from_PRG(int player_no, octetStreams &os_send, const gfpMatrix &shares_prng, octetStream &o_receive);

    // Reveal the aux_sharing-sharing
    gfpMatrix reveal_aux(int degree);

    // Random
    DoubleShareBundle& reduced_random(); // Randomized to the reduced sharings ([r]_t, [r]_2t)
    DoubleShareBundle& truncated_random(ShareBundle &msb);
    DoubleShareBundle& truncated_random(size_t precision);
    DoubleShareBundle& reduced_truncated_random(ShareBundle &msb);
    DoubleShareBundle& reduced_truncated_random(size_t logLearningRate, size_t logMiniBatch);// used in ML
    DoubleShareBundle& reduced_truncated_random(size_t precision);
    DoubleShareBundle& reduced_truncated_random(vector<size_t>& precision);

    DoubleShareBundle& unbounded_prefix_mult_random();
};


}
#endif