/* crypto/aes/aes.h -*- mode:C; c-file-style: "eay" -*- */
/* ====================================================================
 * Copyright (c) 1998-2002 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 */

#ifndef TOOLS_AES_H
#define TOOLS_AES_H
#include "Tools/octetStream.h"

#ifdef __x86_64__
#include <immintrin.h>
#elif __aarch64__
#include "sse2neon.h"
inline __m128i _mm_aesimc_si128(__m128i a) {
	return vreinterpretq_m128i_u8(vaesimcq_u8(vreinterpretq_u8_m128i(a)));
}
inline __m128i _mm_aesdec_si128 (__m128i a, __m128i RoundKey)
{
    return vreinterpretq_m128i_u8(vaesimcq_u8(vaesdq_u8(vreinterpretq_u8_m128i(a), vdupq_n_u8(0)) ^ vreinterpretq_u8_m128i(RoundKey)));
}

inline __m128i _mm_aesdeclast_si128 (__m128i a, __m128i RoundKey)
{
    return vreinterpretq_m128i_u8(vaesdq_u8(vreinterpretq_u8_m128i(a), vdupq_n_u8(0)) ^ vreinterpretq_u8_m128i(RoundKey));
}
#endif

using block = __m128i;
#define AES_BLK_SIZE 16
typedef unsigned int  uint;

void aes_128_schedule( octet* key, const octet* userkey );
void aes_schedule( octet* key, const octet* userkey );
/**
 * @brief Default key schedule is 128-bit key schedule
 * 
 * @param key round keys
 * @param userkey 128-bit seed
 */
inline void aes_schedule( octet* key, const octet* userkey )
{
    aes_128_schedule(key,userkey); 
}

inline __m128i AES_128_ASSIST (__m128i temp1, __m128i temp2) 
{
    __m128i temp3; temp2 = _mm_shuffle_epi32 (temp2 ,0xff); 
    temp3 = _mm_slli_si128 (temp1, 0x4); 
    temp1 = _mm_xor_si128 (temp1, temp3); 
    temp3 = _mm_slli_si128 (temp3, 0x4); 
    temp1 = _mm_xor_si128 (temp1, temp3); 
    temp3 = _mm_slli_si128 (temp3, 0x4); 
    temp1 = _mm_xor_si128 (temp1, temp3); 
    temp1 = _mm_xor_si128 (temp1, temp2); 
    return temp1; 
} 

/**
 * @brief Use AES instructions to generate round keys
 * 
 * @param key 
 * @param userkey 
 */
inline void
#ifdef __x86_64__
__attribute__((target("aes,sse2")))
#endif
aes_128_schedule(octet* key, const octet* userkey)
{
    __m128i temp1, temp2;
    __m128i *Key_Schedule = (__m128i*)key;
    temp1 = _mm_loadu_si128((__m128i*)userkey);
    Key_Schedule[0] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1 ,0x1);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[1] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x2);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[2] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x4);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[3] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x8);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[4] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x10);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[5] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x20);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[6] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x40);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[7] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x80);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[8] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x1b);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[9] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1,0x36);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[10] = temp1;
}

/**
 * @brief AES-ECB mode with N pipelines.
 * (Tell gcc to unroll loops)
 * 
 */
template<int N>
#ifndef __clang__
__attribute__((optimize("unroll-loops")))
#endif
inline void ecb_aes_128_encrypt(__m128i* out, const __m128i* in, const octet* key)
{
    __m128i tmp[N];
    for(int i = 0; i < N; i++){
        tmp[i] = _mm_xor_si128(in[i], ((__m128i*)key)[0]);
    }
        
    int j;
    for(j = 1; j < 10; j++)
        for(int i = 0; i < N; i++){
            tmp[i] = _mm_aesenc_si128(tmp[i], ((__m128i*)key)[j]);
        }
    
    // the last round
    for(int i = 0; i < N; i++){
        out[i] = _mm_aesenclast_si128(tmp[i], ((__m128i*)key)[j]);
    }
}

#endif
