#ifndef TOOLS_INT_H_
#define TOOLS_INT_H_

typedef unsigned char octet;
// Assumes word is a 64 bit value
#ifdef WIN32
  typedef unsigned __int64 word;
#else
  typedef unsigned long word;
#endif

/**
 * @brief positive modulo
 * 
 * @param i 
 * @param n 
 * @return int 
 */
inline int positive_modulo(int i, int n)
{
    return (i%n + n) % n;
}
#endif