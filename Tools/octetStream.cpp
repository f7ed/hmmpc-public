#include <string>

#include "Tools/octetStream.h"
#include "Networking/Player.h"

/**
 * @brief Construct a new octet Stream from a string
 * 
 * @param other other string
 */
octetStream::octetStream(const string& other)
{
    mxlen = other.size();
    len = mxlen;
    ptr = 0;
    data = new octet[mxlen];
    avx_memcpy(data, (const octet*)other.data(), len*sizeof(octet));
}

/**
 * @brief Initial allocation
 * 
 * @param maxlen 
 */
octetStream::octetStream(size_t maxlen)
{
    mxlen = maxlen;
    len = 0;
    ptr = 0;
    data = new octet[mxlen];
}

octetStream::octetStream(size_t len, const octet* source):
    octetStream(len)
{
    append(source, len);
}

// Free the memory
void octetStream::clear()
{
    if(data){
        delete[] data;
    }
    data = 0;
    len = mxlen = ptr = 0;
}

// Assign with other octetStream
void octetStream::assign(const octetStream &os)
{
    if(os.len>mxlen){
        if(data){
            delete[] data;
        }
        mxlen = os.mxlen;
        data = new octet[mxlen];
    }
    len = os.len;
    memcpy(data, os.data, len*sizeof(octet));
    ptr = os.ptr;
}

/**
 * @brief Get string
 * 
 * @return string 
 */
string octetStream::str() const
{
    return string((char*)get_data(), get_length());
}

void octetStreams::reset(size_t num)
{
    resize(num);
    for(auto& o: *this){
        o.reset_write_head();
    }
}

// Free the memory of each octetStream
void octetStreams::clear()
{
    for(auto &o: *this){
        o.clear();
    }
}
