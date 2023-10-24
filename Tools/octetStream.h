#ifndef _octetStream
#define _octetStream

// bytesize of length field
#ifndef LENGTH_SIZE
#define LENGTH_SIZE 4
#endif

#include <cstring>
#include <vector>
#include <cstdio>
#include <iostream>
#include <cassert>

#include "Networking/sockets.h"
#include "Tools/int.h"
#include "Tools/avx_memcpy.h"

using namespace std;
// static int latency_cnt = 0;
class octetStream
{
    // mxlen: max length of data allocation
    // len: write header
    // ptr: read header
    size_t len, mxlen, ptr;
    octet *data;

    void reset() {mxlen = len = ptr = 0; data = 0;}

public:
    octetStream(): len(0), mxlen(0), ptr(0), data(0) {}
    ~octetStream() {if(data){delete[] data;}}

    // Construct a new octet Stream from a string
    octetStream(const string& other);
    // Inital allocation
    octetStream(size_t maxlen);
    // inital buffer
    octetStream(size_t len, const octet* source);
    octetStream& operator=(const octetStream& os)
    { if (this!=&os) { assign(os); }
      return *this;
    }

    // Free memory
    void clear();
    void assign(const octetStream& os);

    // Increase allocation
    void resize_precise(size_t l);/* precise resize to length of l */
    void resize_min(size_t l); /* resize to max(l, mxlen) */
    void resize(size_t l); /* double resize l */
    
    // Covert to string
    string str() const;
    // Get Length
    size_t get_length() const {return len;}
    // Get data pointer
    octet* get_data() const { return data;}
    // Get read pointer
    octet* get_data_ptr() const {return data + ptr;}
    
    // Reset reading head (after receiving data)
    void reset_read_head() {ptr = 0;}
    // Reset writing and reading heads (before receiving)
    void reset_write_head() {len = 0; ptr = 0;}

    // Append 'l' bytes from 'x'
    void append(const octet* x, const size_t l);
    octet* append(const size_t l);

    // Read the data
    octet* consume(size_t l);// Return the read pointer and update read head.
    void consume(octet* x, const size_t l);// Read l octets to x.

    // Append integer of "n_bytes" n_bytes
    void store_int(size_t a, int n_bytes);
    // Read integer of "n_bytes" bytes
    size_t get_int(int n_bytes);

    void store(size_t a) { store_int(a, 8); }
    void get(size_t &a) {a = get_int(8); }

    /**
     * @brief Construct send package and send it via the socket.
     * Package = [len||data]
     * length field size is 4 bytes
     * @param socket_num socket number
     */
    void Send(int socket_num)const;
    // Receive on socket_num
    void Receive(int socket_num);
};

class Player;

class octetStreams : public vector<octetStream>
{
public:
    octetStreams(){}
    octetStreams(size_t num){reset(num);}
    void reset(size_t num);
    void clear();
};

/**
 * @brief Resize the data allocation to size l
 * 
 * @param l 
 */
inline void octetStream::resize_precise(size_t l)
{
    if (l == mxlen){
        return;
    }
    // New allocation
    octet *nd = new octet[l];
    if (data)
    {
        // copy the origin data to new memory
        memcpy(nd, data, min(len, l)*sizeof(octet));
        delete [] data;
    }
    data = nd;
    mxlen = l;
}

/**
 * @brief Resize the data allocation at least l
 * 
 * @param l 
 */
inline void octetStream::resize_min(size_t l)
{
    if (l > mxlen){
        resize_precise(l);
    }
}

/**
 * @brief double resize if needed
 * 
 * @param l 
 */
inline void octetStream::resize(size_t l)
{
    if (l < mxlen){
        return;
    }
    resize_precise(l*2);
}

/**
 * @brief Get the write ptr
 * 
 * @param l append length
 * @return octet* 
 */
inline octet* octetStream::append(const size_t l)
{
    if (len+l > mxlen){
        resize(len+l);
    }
    // update write header
    octet* res = data + len;
    len+=l;
    return res;
}

/**
 * @brief Append x
 * 
 * @param x source ptr
 * @param l length
 */
inline void octetStream::append(const octet* x, const size_t l)
{
    // append(l): get write ptr
    avx_memcpy(append(l), x, l*sizeof(octet));
}

/**
 * @brief Consume(Read) the data in octetStream
 * 
 * @param l Length bytes to read.(update the read head)
 * @return octet* Return the read pointer.
 */
inline octet* octetStream::consume(size_t l)
{
    if(ptr+l>len){
        throw runtime_error("insufficient data");
    }
    octet* res = data+ptr;
    ptr+=l; //update the read header
    return res;// return the read header
}

inline void octetStream::consume(octet* x, const size_t l)
{
    avx_memcpy(x, consume(l), l*sizeof(octet));
}

inline void octetStream::store_int(size_t l, int n_bytes)
{
    resize(len+l);
    encode_length(data+len, l, n_bytes);
    len+=n_bytes;
}

inline size_t octetStream::get_int(int n_bytes)
{
    return decode_length(consume(n_bytes), n_bytes);
}

/**
 * @brief Receive the package via the socket.
 * Package = [len||data]
 * length field is 4 bytes
 * @param socket_num connection socket number
 */
inline void octetStream::Receive(int socket_num)
{
    // read the length field: nlen
    size_t nlen = 0;
    receive(socket_num, nlen, LENGTH_SIZE);
    len = 0;
    // resize the data
    resize_min(nlen);
    len = nlen;
    receive(socket_num, data, len);
    reset_read_head();
}

/**
 * @brief Construct send package and send it via the socket.
 * Package = [len||data].
 * Length field size is 4 bytes
 * @param socket_num connection socket number
 */
inline void octetStream::Send(int socket_num)const
{
    // send length
    send(socket_num, len, LENGTH_SIZE);
    // send data
    send(socket_num, data, len);
}

#endif