#include "Protocols/RandomShare.h"
#include "Protocols/Bit.h"
#include <vector>

using Eigen::seq, Eigen::seqN, Eigen::last;
namespace hmmpc
{

// The queue store the preprocessed random share, which is nothing to do with the input of the party.
queue<gfpScalar> RandomShare::queueRandom; // [r]_t
queue<gfpScalar> RandomShare::queueRandomBit;

queue<gfpScalar> DoubleRandom::queueReducedRandom; // [r]_t, [r]_2t
queue<gfpScalar> DoubleRandom::queueTruncatedRandom; // [r/2^d]_t, [r]_t
queue<gfpScalar> DoubleRandom::queueReducedTruncatedRandom; // [r/2^d]_t, [r]_2t
queue<gfpScalar> DoubleRandom::queueUnboundedMultRandom; // ([b1], [b1^-1]), ([bi], [bi-1 * bi^-1]) for i = 2, ..., l. 
queue<gfpScalar> DoubleRandom::queueTruncatedRandomInML;
queue<gfpScalar> DoubleRandom::queueReducedTruncatedInML;
queue<gfpScalar> DoubleRandom::queueReducedTruncatedWithPrecisionRandom;
/************************************************************************
 * 
 *       Definition of static member functions about RandomShare
 * 
 * **********************************************************************/
void RandomShare::get_random(queue<gfpScalar> &Q, gfpScalar &res)
{
    assert(Q.size()>0);
    res = Q.front();
    Q.pop();
    return;
}

void RandomShare::get_randoms(queue<gfpScalar> &Q, gfpMatrix &res)
{
    size_t num = res.rows()*res.cols();
    assert(num <= Q.size());
    for(size_t i = 0; i < num; i++){
        res(i) = Q.front();
        Q.pop();
    }
    return;
}

/**
 * @brief Generate num (at least num) random sharings using DN07 RandomShare Protocol.
 * Every execution of the RandomShare Protocol will generate (n-t) random sharings where n is number of players and t is the threshold.
 * 
 * @param num 
 */
void RandomShare::generate_random_sharings(size_t num)
{
    size_t bundle_size = bundles(num); // ceil( num / (n_player - threshold ))

    ShareBundle crude_inputs(n_players, bundle_size);// Each row corresponds the input of one party.
    // Input random sharing respectively from each party.
    vector<ShareBundle> individual_input(n_players);
    octetStreams os_send(n_players), os_receive(n_players);
    for(size_t i = 0; i < n_players; i++){
        individual_input[i].resize(1, bundle_size);
        // Each party only requests to distribute the sharings 
        // so that all the parties could start to distribute sharings as soon as possible. 
        individual_input[i].input_from_random_request(i, os_send, os_receive[i]);
    }

    // The multi-thread version.
    // The distributer should wait to send all the sharings.
    // The other receivers should wait to receive the shares.
    for(size_t i = 0; i < n_players; i++){
        individual_input[i].finish_input_from(i, os_send, os_receive[i]);
        crude_inputs.set_row(i, individual_input[i]);
    }
    
    // Use vandermonde matrix to extract randomness.
    gfpMatrix extracted_random = vandermonde_n_t.transpose()*crude_inputs.shares;
    for(size_t i = 0; i < extracted_random.size(); i++){
        queueRandom.push(extracted_random(i));
    }
}


/**
 * @brief Generate a bundle of sharings of bits into the queueRandomBit queue.
 * 
 * @param num 
 */
void RandomShare::generate_random_bits(size_t num)
{
    ShareBundle R(num, 1);
    R.random();
    gfpMatrix &r = R.shares;
    
    ShareBundle r_square(num, 1);
    r_square.shares = square(r.array()); // *BUG LOG: We cannot use r.array().square() since we still use the origin r.
    
    // TODO: Modify with peusodo-random sharing of zero. 
    // In temporary, we use this unsecure operation but with the same complexity. (since PRSZ costs free)
    r_square.double_degree();
    gfpMatrix s = r_square.reveal();

    // r_square.reduce_degree(); // * BUG LOG: Remember to reduce degree when multiplying two shares of degree t. 
    // gfpMatrix s = r_square.reveal();
    for(size_t i = 0; i < s.size(); i++){
        if(s(i) == 0){ // TODO resample
            throw abort_error("[RandomShare]: get_random_bits samples 0");
        }
    }
    
    // gfpMatrix r_prime = s.array().rsqrt();
    // Use Batch Inversion in rsqrt
    gfpMatrix r_sqrt = s.array().sqrt();
    gfpMatrix r_prime(r_sqrt.rows(), r_sqrt.cols());
    batch_inversion(r_sqrt, r_prime);

    gfpMatrix res = ((r.array() * r_prime.array() ) + 1)/2;
    for(size_t i = 0; i < num; i++){
        queueRandomBit.push(res(i));
    }
    
    return;
}

// Replace with input_from_random_
void RandomShare::generate_random_sharings_PRG(size_t num)
{
    size_t bundle_size = bundles(num); // ceil( num / (n_player - threshold ))

    ShareBundle crude_inputs(n_players, bundle_size);// Each row corresponds the input of one party.
    
    // Input random sharing respectively from each party.
    vector<ShareBundle> individual_input(n_players);
    octetStreams os_send(n_party_PRG()), os_receive(n_players);
    vector<gfpMatrix> shares_prng(n_players);

    for(int i = 0; i < n_players; i++){
        individual_input[i].resize(1, bundle_size);
        // Each party only requests to distribute the sharings 
        // so that all the parties could start to distribute sharings as soon as possible. 
        individual_input[i].input_from_random_request_PRG(i, os_send, shares_prng[i], os_receive[i]);
    }

    // The multi-thread version.
    // The distributer should wait to send all the sharings.
    // The other receivers should wait to receive the shares.
    for(int i = 0; i < n_players; i++){
        individual_input[i].finish_input_from_PRG(i, os_send, shares_prng[i], os_receive[i]);
        crude_inputs.set_row(i, individual_input[i]);
    }

    // Use vandermonde matrix to extract randomness.
    gfpMatrix extracted_random = vandermonde_n_t.transpose()*crude_inputs.shares;
    for(size_t i = 0; i < extracted_random.size(); i++){
        queueRandom.push(extracted_random(i));
    }
    return;
}

/************************************************************************
 * 
 *       Definition of static member functions about DoubleRandom
 * 
 * **********************************************************************/

void DoubleRandom::get_random_pair(queue<gfpScalar>&Q, gfpScalar &r, gfpScalar &aux_r)
{
    assert(Q.size()>=2);
    r = Q.front();
    Q.pop();
    aux_r = Q.front();
    Q.pop();
    return;
}


void DoubleRandom::get_random_pairs(queue<gfpScalar>&Q, gfpMatrix &r, gfpMatrix &aux_r)
{
    assert(r.size()*2 <= Q.size());
    for(size_t i = 0; i < r.size(); i++){
        r(i) = Q.front();
        Q.pop();
        aux_r(i) = Q.front();
        Q.pop();
    }
    return;
}

void DoubleRandom::get_random_triple(queue<gfpScalar>&Q, gfpScalar &r, gfpScalar &aux_r, gfpScalar &sub_r)
{
    assert(Q.size()>=3);
    r = Q.front();
    Q.pop();
    aux_r = Q.front();
    Q.pop();
    sub_r = Q.front();
    Q.pop();
    return;
}

void DoubleRandom::get_random_triples(queue<gfpScalar> &Q, gfpMatrix &r, gfpMatrix &aux_r, gfpMatrix &sub_r)
{
    assert(r.size()*3 <= Q.size());
    for(size_t i = 0; i < r.size(); i++){
        r(i) = Q.front();
        Q.pop();
        aux_r(i) = Q.front();
        Q.pop();
        sub_r(i) = Q.front();
        Q.pop();
    }
    return;
}
/**********************************************************
 * *      Reduced Random Sharings - ( [r]_t, [r]_2t )
 * *********************************************************/
/**
 * @brief Generate num (at least num) double random sharings using DN07 DoubleRandom Protocol.
 * Every execution of the DoubleRandom Protocol will generate (n-t) double random sharings where n is number of players and t is the threshold.
 * Each pair of double random sharing consists of ([r]_t, [r]_2t)
 * 
 * @param num 
 */
void DoubleRandom::generate_reduced_random_sharings(size_t num)
{
    size_t bundle_size = bundles(num);
    DoubleShareBundle crude_inputs(n_players, bundle_size); // Each row corresponds to one party's input
    // Input reduced random sharing respectively from each party
    vector<DoubleShareBundle> individual_input(n_players);
    octetStreams os_send(n_players), os_receive(n_players);
    for(size_t i = 0; i < n_players; i++){
        individual_input[i].resize(1, bundle_size);
        // Each party only requests to distribute the sharings 
        // so that all the parties could start to distribute sharings as soon as possible. 
        individual_input[i].input_from_random_request(i, os_send, os_receive[i]);
    }

    // The multi-thread version.
    // The distributer should wait to send all the sharings.
    // The other receivers should wait to receive the shares.
    for(size_t i = 0; i < n_players; i++){
        individual_input[i].finish_input_from(i, os_send, os_receive[i]);
        crude_inputs.set_row(i, individual_input[i]);
    }

    // We can calculate extracted_t_random and extracted_2t_random in one matrix multiplication.
    // *Original matrix multiplication
    // gfpMatrix extracted_t_random = vandermonde_n_t.transpose() * crude_inputs.shares;
    // gfpMatrix extracted_2t_random = vandermonde_n_t.transpose() * crude_inputs.aux_shares; 
    // for(size_t i = 0; i < extracted_t_random.size(); i++){
    //     queueReducedRandom.push(extracted_t_random(i));
    //     queueReducedRandom.push(extracted_2t_random(i));
    // }


    // Use vandermonde matrix to extract randomness
    // We incorporate two matrix multiplication into one matrix multiplication.
    gfpMatrix extracted_random = vandermonde_n_t.transpose() * 
        (gfpMatrix(n_players, bundle_size<<1)<<crude_inputs.shares, crude_inputs.aux_shares).finished();
    
    for(size_t i = 0; i < extracted_random.rows(); i++){
        // j indicates the t-sharing and k indicates the 2t-sharing
        for(size_t j = 0, k = bundle_size; k < extracted_random.cols(); j++, k++){
            queueReducedRandom.push(extracted_random(i, j));
            queueReducedRandom.push(extracted_random(i, k));
        }
    }
    
}

// Replace with input_from_random_request_PRG.
void DoubleRandom::generate_reduced_random_sharings_PRG(size_t num)
{
    size_t bundle_size = bundles(num);
    DoubleShareBundle crude_inputs(n_players, bundle_size); // Each row corresponds to one party's input
    // Input reduced random sharing respectively from each party

    // * The part that is replaced.
    vector<DoubleShareBundle> individual_input(n_players);
    octetStreams os_send(n_players), os_receive(n_players);
    vector<gfpMatrix>shares_prng(n_players);

    for(int i = 0; i < n_players; i++){
        individual_input[i].resize(1, bundle_size);
        // Each party only requests to distribute the sharings 
        // so that all the parties could start to distribute sharings as soon as possible.
        individual_input[i].input_from_random_request_PRG(i, os_send, shares_prng[i], os_receive[i]); 
    }

    for(int i = 0; i < n_players; i++){
        individual_input[i].input_aux_from_random_PRG(i);
    }

    for(int i = 0; i < n_players; i++){
        individual_input[i].finish_input_from_PRG(i, os_send, shares_prng[i], os_receive[i]);
        crude_inputs.set_row(i, individual_input[i]);
    }
    // *

    gfpMatrix extracted_random = vandermonde_n_t.transpose() * 
        (gfpMatrix(n_players, bundle_size<<1)<<crude_inputs.shares, crude_inputs.aux_shares).finished();
    
    for(size_t i = 0; i < extracted_random.rows(); i++){
        // j indicates the t-sharing and k indicates the 2t-sharing
        for(size_t j = 0, k = bundle_size; k < extracted_random.cols(); j++, k++){
            queueReducedRandom.push(extracted_random(i, j));
            queueReducedRandom.push(extracted_random(i, k));
        }
    }
}

/**********************************************************
 * *      Truncated Random Sharings - ( [r/2^d]_t, [r]_t , [r_{msb}]_t)
 * *********************************************************/
void DoubleRandom::generate_truncated_random_sharings(size_t num)
{
    BitBundle bits(num, BITS_LENGTH);
    bits.random();
    
    gfpMatrix trunc_bits(num, BITS_LENGTH);
    trunc_bits.leftCols(INT_PRECISION) = bits.shares.rightCols(INT_PRECISION);
    for(size_t i = INT_PRECISION; i < BITS_LENGTH; i++){
        // Fill the empty bits with the MSB of the original bits
        trunc_bits.col(i) = bits.shares.col(BITS_LENGTH - 1);
    }

    // *Original matrix multiplication
    // gfpMatrix res = bits.shares * bits_coeff;
    // gfpMatrix res_trunc = trunc_bits * bits_coeff; 

    // We incorporate two matrix multiplication into one matrix multiplication.
    gfpMatrix res = (gfpMatrix(num<<1, BITS_LENGTH)<< trunc_bits, bits.shares).finished() * bits_coeff;
    for(size_t i = 0, j = num; i < num; i++, j++){
        queueTruncatedRandom.push(res(i));
        queueTruncatedRandom.push(res(j));
        queueTruncatedRandom.push(bits.shares(i, BITS_LENGTH-1));// msb
    }
}

// Truncate with specific precision
void DoubleRandom::generate_truncated_random_sharings(size_t num, size_t precision)
{
    BitBundle bits(num, BITS_LENGTH);
    bits.random();
    
    size_t int_precision = BITS_LENGTH - precision;
    gfpMatrix trunc_bits(num, BITS_LENGTH);
    trunc_bits.leftCols(int_precision) = bits.shares.rightCols(int_precision);
    for(size_t i = int_precision; i < BITS_LENGTH; i++){
        // Fill the empty bits with the MSB of the original bits
        trunc_bits.col(i) = bits.shares.col(BITS_LENGTH - 1);
    }

    // We incorporate two matrix multiplication into one matrix multiplication.
    gfpMatrix res = (gfpMatrix(num<<1, BITS_LENGTH)<< trunc_bits, bits.shares).finished() * bits_coeff;
    for(size_t i = 0, j = num; i < num; i++, j++){
        queueTruncatedRandomInML.push(res(i));
        queueTruncatedRandomInML.push(res(j));
    }
}


/**********************************************************************
 * *      Reduced Truncated Random Sharings - ( [r/2^d]_t, [r]_2t , [r_msb]_t)
 * *********************************************************************/
void DoubleRandom::generate_reduced_truncated_random_sharings(size_t num)
{
    BitBundle bitsBundle(num, BITS_LENGTH);
    bitsBundle.random();
    gfpMatrix &bits = bitsBundle.shares;
    gfpMatrix trunc_bits(num, BITS_LENGTH);
    trunc_bits.leftCols(INT_PRECISION) = bits.rightCols(INT_PRECISION);
    for(size_t i = INT_PRECISION; i < BITS_LENGTH; i++){
        // Fill the empty bits with the MSB of the original bits
        trunc_bits.col(i) = bits.col(BITS_LENGTH - 1);
    }
    bits.array().square(); // Each bit turns to 2t-sharing
    gfpMatrix res = (gfpMatrix(num<<1, BITS_LENGTH)<< trunc_bits, bits).finished() * bits_coeff;

    for(size_t i = 0, j = num; i < num; i++, j++){
        queueReducedTruncatedRandom.push(res(i));
        queueReducedTruncatedRandom.push(res(j));
        queueReducedTruncatedRandom.push(trunc_bits(i, BITS_LENGTH-1));// msb
    }
}

void DoubleRandom::generate_reduced_truncated_random_sharings(queue<gfpScalar> &Q, size_t num, size_t precision)
{
    BitBundle R(num, BITS_LENGTH);
    R.random();
    gfpMatrix &bits = R.shares;

    gfpMatrix trunc_bits(num, BITS_LENGTH);
    size_t int_precision = BITS_LENGTH - precision;
    trunc_bits.leftCols(int_precision) = bits.rightCols(int_precision);
    for(size_t i = int_precision; i < BITS_LENGTH; i++){
        // Fill the empty bits with the MSB of the original bits
        trunc_bits.col(i) = bits.col(BITS_LENGTH - 1);
    }

    bits.array().square(); // Each bit turns to 2t-sharing
    gfpMatrix res = (gfpMatrix(num<<1, BITS_LENGTH)<< trunc_bits, bits).finished() * bits_coeff;

    for(size_t i = 0, j = num; i < num; i++, j++){
        Q.push(res(i));
        Q.push(res(j));
    }
}

// Generate reduced and truncated random sharings with different precision.
// num_repetitions: the number of repetitions
void DoubleRandom::generate_reduced_truncated_random_sharings(queue<gfpScalar> &Q, size_t num, vector<size_t> &precision, size_t num_repetitions)
{
    size_t total = num*num_repetitions;
    BitBundle R(total, BITS_LENGTH);
    R.random();
    gfpMatrix &bits = R.shares;

    gfpMatrix trunc_bits(total, BITS_LENGTH);
    vector<size_t> int_precision;
    for(size_t i = 0; i < num_repetitions; i++){
        for(size_t j = 0; j < num; j++){
            int_precision.push_back(BITS_LENGTH - precision[j]);
        }
    }

    for(size_t i = 0; i < total; i++){
        trunc_bits.row(i).leftCols(int_precision[i]) = bits.row(i).rightCols(int_precision[i]);
        trunc_bits.row(i).rightCols(BITS_LENGTH - int_precision[i]).setConstant(bits(i, BITS_LENGTH-1));
    }

    bits.array().square(); // Each bit turns to 2t-sharing
    gfpMatrix res = (gfpMatrix(total<<1, BITS_LENGTH)<< trunc_bits, bits).finished() * bits_coeff;

    for(size_t i = 0, j = total; i < total; i++, j++){
        Q.push(res(i));
        Q.push(res(j));
    }
}


/**********************************************************************
 * *      Random Sharings for Unbounded Multiplication 
 * An unbounded multiplication instance
 * ( [b1]_t , [b1^-1]_t )
 * ( [b2]_t , [b1 * b2^-1]_t,)
 * ( [b3]_t , [b2 * b3^-1]_t)
 * ...
 * ( [bi]_t , [bi-1 * bi^-1]_t) for i = 2, ..., l
 * *********************************************************************/
// One unbounded-mult instance.
// The following is unbounded multiplication in parallel, which is more practical.
void DoubleRandom::generate_unbounded_random_sharings(size_t num)
{
    ShareBundle R(num, 2);
    R.random();
    gfpMatrix &b_bPrime = R.shares;
    gfpMatrix tmp_b_bPrime(num+num-1, 2); // (bi, bi')
    
    tmp_b_bPrime(seqN(0, num), seq(0, last)) = b_bPrime;
    tmp_b_bPrime.col(0)(seqN(num, num-1)) = b_bPrime.col(0)(seqN(0, num-1));
    tmp_b_bPrime.col(1)(seqN(num, num-1)) = b_bPrime.col(1)(seqN(1, num-1));
    // tmp_b_bPrime(seqN(num, num-1), seq(0, 0)) = b_bPrime(seqN(0, num-1), seq(0, 0));
    // tmp_b_bPrime(seqN(num, num-1), seq(1, 1)) = b_bPrime(seqN(1, num-1), seq(1, 1));
    
    ShareBundle prod(num+num-1, 1);
    prod.shares = tmp_b_bPrime.col(0).array() * tmp_b_bPrime.col(1).array();
    prod.reduce_degree();

    ShareBundle B(num, 1);// Bi = bi * bi'
    B.shares = prod.shares(seqN(0, num), seq(0, last));
    B.reveal();

    // gfpMatrix B_inv = B.secret().array().inverse(); // B_inv 
    // *Use Batch Inversion
    gfpMatrix B_inv(B.rows(), B.cols());
    batch_inversion(B.secret(), B_inv);

    gfpVector prod_b(num);
    prod_b(0) = b_bPrime(0, 1); // b0'
    prod_b(seqN(1, num-1)) = prod.shares(seqN(num, num-1), seqN(0, 1));

    gfpVector res = B_inv.array() * prod_b.array();
    for(size_t i = 0; i < num; i++){
        queueUnboundedMultRandom.push(b_bPrime(i, 0));
        queueUnboundedMultRandom.push(res(i));
    }
}

// Each row corresponds to an unbounded multiplication instance.
void DoubleRandom::generate_unbounded_random_sharings(size_t xSize, size_t ySize)
{
    ShareBundle R(xSize<<1, ySize);
    R.random();
    gfpMatrix &rand = R.shares;

    gfpMatrix rand_b(xSize, ySize+ySize-1), rand_b_prime(xSize, ySize+ySize-1);

    // rand_b = b | b
    // rand_b = b'| b'
    rand_b(seqN(0, xSize), seqN(0, ySize)) = rand(seqN(0, xSize), seqN(0, ySize));
    rand_b(seqN(0, xSize), seqN(ySize, ySize-1)) = rand(seqN(0, xSize), seqN(0, ySize-1));
    rand_b_prime(seqN(0, xSize), seqN(0, ySize)) = rand(seqN(xSize, xSize), seqN(0, ySize));
    rand_b_prime(seqN(0, xSize), seqN(ySize, ySize-1)) = rand(seqN(xSize, xSize), seqN(1, ySize-1));

    ShareBundle prod(xSize, ySize+ySize-1);
    prod.shares = rand_b.array() * rand_b_prime.array();
    prod.reduce_degree();

    ShareBundle B(xSize, ySize); // B = mult_cwise(b, b')
    B.shares = prod.shares(seqN(0, xSize), seqN(0, ySize));
    B.reveal();
    // gfpMatrix B_inv =  B.secret().array().inverse(); // B_inv //TODO: Evaluate the ex_gcd and pow to implement inverse. 
                                                    // BUG LOG: inverse() dose not return the reference.
    // *Use Batch Inversion
    gfpMatrix B_inv(B.rows(), B.cols());
    batch_inversion(B.secret(), B_inv);

    gfpMatrix X(xSize, ySize);
    X.col(0) = rand_b_prime.col(0); // b'
    X(seqN(0, xSize), seqN(1, ySize-1)) = prod.shares(seqN(0, xSize), seqN(ySize, ySize-1));
    
    gfpMatrix res(xSize, ySize);
    res = B_inv.array() * X.array();
    for(size_t i = 0; i < xSize; i++)
        for(size_t j = 0; j < ySize; j++){
            queueUnboundedMultRandom.push(rand_b(i, j));
            queueUnboundedMultRandom.push(res(i, j));
        }
    return;
}


}

