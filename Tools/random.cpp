#include <iostream>
#include "Tools/random.h"

using namespace std;

PRNG::PRNG():
    cnt(0)
{

}

/**
 * @brief Construct a new PRNG::PRNG object from octetStream to get seed
 * 
 * @param seed octetStream
 */
PRNG::PRNG(octetStream& seed):PRNG()
{
    SetSeed(seed.consume(SEED_SIZE));
}

void PRNG::ReSeed()
{
    randombytes_buf(seed, SEED_SIZE);
    InitSeed();
}

/**
 * @brief Set seed from octetStream
 * 
 * @param inp 
 */
void PRNG::SetSeed(const octet* inp)
{
    memcpy(seed, inp, SEED_SIZE*sizeof(octet));
    InitSeed();
}

/**
 * @brief Use AES-CTR
 * 
 */
void PRNG::InitSeed()
{
#ifdef USE_AES
    aes_schedule(KeySchedule, seed);
    #ifdef DEBUG_RANDOM
    cout<<"PIPELINES: "<<PIPELINES<<endl;
    #endif
    memset(state, 0, RAND_SIZE*sizeof(octet));
    // Initial the state
    for(int i = 0; i < PIPELINES; i++){
        state[i*AES_BLK_SIZE] = i;
    }
#else
    memcpy(state, seed, SEED_SIZE*sizeof(octet));
#endif
    hash();
#ifdef DEBUG_RANDOM
    cout<<"[SetSeed]:";
    print_state();
    cout<<endl;
#endif
}

/**
 * @brief Use AES-ECB to hash to state
 * 
 */
void PRNG::hash()
{
    ecb_aes_128_encrypt<PIPELINES>((__m128i*)random,(__m128i*)state,KeySchedule);
    // This is a new random value so we have not used any of it yet
    cnt=0;
}

/**
 * @brief Increment state
 * 
 */
void PRNG::next()
{
    for(int i = 0; i < PIPELINES; i++){
        // the lower part of state
        int64_t* s = (int64_t*)&state[i*AES_BLK_SIZE];
        s[0] += PIPELINES;
        // carry the higher part
        if(s[0]==0){s[1]++;}
    }
    hash();
}

unsigned char PRNG::get_uchar()
{
  if (cnt>=RAND_SIZE) { next(); }
  unsigned char ans=random[cnt];
  cnt++;
  // print_state(); cout << " UCHA " << (int) ans << endl;
  return ans;
}

unsigned int PRNG::get_uint()
{
    if(cnt>RAND_SIZE-4){
        next();
    }
    unsigned int a0=random[cnt],a1=random[cnt+1],a2=random[cnt+2],a3=random[cnt+3];
    cnt = cnt+4;
    unsigned int ans=(a0+(a1<<8)+(a2<<16)+(a3<<24));
    return ans;
}

bool PRNG::get_bit()
{
    if(n_cached_bits == 0){
        cached_bits = get_word();
        n_cached_bits = 64;
    }
    n_cached_bits--;
    return (cached_bits>> n_cached_bits) & 1;
}

void PRNG::print_state() const
{
    unsigned i;
    // cout seed
    for(i = 0; i < SEED_SIZE; i++)
    {
        if(seed[i]<10){cout<<"0";}
        cout<<hex<<(int)seed[i];
    }
    cout<<"\t";
    // cout random
    for(int i = 0; i < RAND_SIZE; i++)
    {
        if(random[i]<10){cout<<"0";}
        cout<<hex<<(int)random[i];
    }
    cout<<"\t";
    // cout state
    for (i=0; i<RAND_SIZE; i++)
    { 
        if (state[i]<10) { cout << "0"; }
        cout << hex << (int) state[i];
    }
    cout << "Cnt: " << dec << cnt <<endl;
}

