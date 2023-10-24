#ifndef _random
#define _random

#include <sodium.h>
#include "Tools/aes.h"
#include "Tools/avx_memcpy.h"
#include "Networking/data.h"
#include "Tools/octetStream.h"

#define USE_AES

#ifndef USE_AES
    #define PIPELINES 1
    #define SEED_SIZE randombytes_SEEDBYTES
    #define RAND_SIZE 480
#else
    #if defined(__AES__) || !defined(__x86_64__)
        #define PIPELINES 8
    #else
        #define PIPELINES 1
    #endif

    #define SEED_SIZE AES_BLK_SIZE
    #define RAND_SIZE (PIPELINES * AES_BLK_SIZE)
#endif

using namespace std;
// PRNG: Pseudorandom number generator
class PRNG
{

    // Use AES: seed(key), state(in), random(out)
    octet seed[SEED_SIZE];
    octet state[RAND_SIZE] __attribute__((aligned(16)));
    octet random[RAND_SIZE] __attribute__((aligned (16)));
    // 10 round keys (128bit)
    octet KeySchedule[176] __attribute__((aligned (16)));// __attribute__ specify the alignment
    
    const static bool useC=false;// Use instructions to implement AES

    // cnt: read head
    int cnt; // How many bytes of the current random value have been used

    int n_cached_bits;
    word cached_bits;

    void hash();// Hashes state to random and sets cnt=0
    void next();// Increse the State

public:
    PRNG();
    PRNG(octetStream &seed);

    
    void ReSeed();// Set seed from dev/random(from sodium)
    void SetSeed(const octet*);// Set Seed from octetStream
    
    void InitSeed();// Generate round keys from seed

    // For debugging
    void print_state() const;

    // Get Random-*
    word get_word()
    {
        word a;
        get_octets<sizeof(a)>((octet*)&a);
        return le64toh(a);
    }
    unsigned char get_uchar();
    unsigned int get_uint();
    bool get_bit();

    void get_octets(octet* ans, int len);
    template<int L>
    void get_octets(octet* ans);
};

class SeededPRNG: public PRNG
{
public:
    SeededPRNG(){
        ReSeed();
    }
};

/**
 * @brief Get the random octets of length len
 * 
 * @param ans [out]
 * @param len [in]
 */
inline void PRNG::get_octets(octet* ans, int len)
{
    int pos = 0;// write header
    while(len){
        int step = min(len, RAND_SIZE-cnt);
        memcpy(ans+pos, random+cnt, step);
        pos+=step;
        len-=step;
        cnt+=step;
        if(cnt==RAND_SIZE){
            next();
        }
    }
}

template<int L>
inline void PRNG::get_octets(octet* ans)
{
    if(L < RAND_SIZE - cnt){
        avx_memcpy<L>(ans, random+cnt);
        cnt+=L;
    }
    else{
        get_octets(ans, L);
    }
}
#endif