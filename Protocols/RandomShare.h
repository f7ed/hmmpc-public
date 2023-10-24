#ifndef PROTOCOLS_RANDOM_SHARE_H_
#define PROTOCOLS_RANDOM_SHARE_H_

#include "Protocols/Share.h"
#include "Protocols/ShareBundle.h"

namespace hmmpc
{

class RandomShare: public ShareBase
{
protected:
    static size_t bundles(size_t num);

public:

    // Queues to store the preprocessed random sharings
    static queue<gfpScalar> queueRandom; // [r]_t
    static queue<gfpScalar> queueRandomBit;

    static void generate_random_sharings(size_t num); // Output into the queue
    static void generate_random_bits(size_t num); //Output into the queue
    
    static void generate_random_sharings_PRG(size_t num);
    
    static void get_random(queue<gfpScalar> &Q, gfpScalar &res);
    static void get_randoms(queue<gfpScalar> &Q, gfpMatrix &res);
    
};

inline size_t RandomShare::bundles (size_t num)
{
    size_t res = num / (n_players - threshold);
    if(num % (n_players-threshold)){res++;}
    return res;
}

class DoubleRandom: RandomShare
{
public:
    // Queue to store the preprocessed double sharings in order.
    static queue<gfpScalar> queueReducedRandom; // [r]_t, [r]_2t
    static queue<gfpScalar> queueTruncatedRandom; // [r/2^d]_t, [r]_t
    static queue<gfpScalar> queueTruncatedRandomInML; // truncation: learning rate 2^-5 or 2^-7 ; batch size /2^7
    static queue<gfpScalar> queueReducedTruncatedRandom; // [r/2^d]_t, [r]_2t
    static queue<gfpScalar> queueReducedTruncatedInML; // reduced + truncation: 2^-d; learning rate 2^-5 or 2^-7 ; batch size /2^7
    static queue<gfpScalar> queueUnboundedMultRandom; //([b1], [b1^-1]), ([bi], [bi-1 * bi^-1]) for i = 2, ..., l. 
    
    static queue<gfpScalar> queueReducedTruncatedWithPrecisionRandom; // For variable precision.

    // Output into the queue
    static void generate_reduced_random_sharings(size_t num); // [r]_t, [r]_2t
    static void generate_truncated_random_sharings(size_t num); // [r/2^d]_t, [r]_t
    static void generate_truncated_random_sharings(size_t num, size_t precision);
    static void generate_reduced_truncated_random_sharings(size_t num); // [r/2^d]_t, [r]_2t
    static void generate_reduced_truncated_random_sharings(queue<gfpScalar> &Q, size_t num, size_t precision); // [r/2^p]_t, [r]_2t
    static void generate_reduced_truncated_random_sharings(queue<gfpScalar> &Q, size_t num, vector<size_t> &precision, size_t num_repetitions);// different precision
    
    static void generate_unbounded_random_sharings(size_t num); // ([b1], [b1^-1]), ([bi], [bi-1 * bi^-1]) for i = 2, ..., l
    static void generate_unbounded_random_sharings(size_t xSize, size_t ySize); // Each row corresponds an instance of unbounded prefix mult

    static void generate_reduced_random_sharings_PRG(size_t num);

    static void get_random_pair(queue<gfpScalar> &Q, gfpScalar &r, gfpScalar &aux_r);
    static void get_random_pairs(queue<gfpScalar> &Q, gfpMatrix &r, gfpMatrix &aux_r);
    
    static void get_random_triple(queue<gfpScalar>&Q, gfpScalar &r, gfpScalar &aux_r, gfpScalar &sub_r);
    static void get_random_triples(queue<gfpScalar> &Q, gfpMatrix &r, gfpMatrix &aux_r, gfpMatrix &sub_r);
};

}
#endif