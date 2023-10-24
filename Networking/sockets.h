#ifndef _sockets_h
#define _sockets_h

#include <iostream>
#include <unistd.h>
#include <errno.h>      /* Errors */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <arpa/inet.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>   /* Wait for Process Termination */

#include "Tools/int.h"
#include "Tools/Exceptions.h"
#include "Networking/data.h"

using namespace std;

void error(const char *str);
// Client connects to the server(hostname&Portnum) to get connection socket
void set_up_client_socket(int& mysocket, const char* hostname, int Portnum);
// Client closes the connection sockets.
void close_client_socket(int socket);

void send(int& socket, size_t a, size_t len);
void send(int socket, octet *msg, size_t len);

void receive(int& socket, size_t& a, size_t len);
void receive(int socket, octet *msg, size_t len);

/**
 * @brief Send Package's Length Field via the socket.
 * 
 * @param socket socket number
 * @param a the ength number
 * @param len Length Field is 4 bytes
 */
inline void send(int& socket, size_t a, size_t len)
{
    octet blen[len];
    encode_length(blen, a, len);
    send(socket, (octet*)blen, len);
}

/**
 * @brief subroutine of sending data
 * 
 * @param socket 
 * @param msg 
 * @param len 
 * @return size_t 
 */
inline size_t send_non_blocking(int socket, octet *msg, size_t len)
{
    int j = send(socket, msg, len, MSG_DONTWAIT);
    if (j < 0){
        if (errno != EINTR and errno != EAGAIN and errno != EWOULDBLOCK){
            error("Send error -1");
        }
        else{
            return 0;
        }
    }
    return j;
}

/**
 * @brief Send any data via the socket
 * 
 * @param socket socket number
 * @param msg* msg 
 * @param len msg length
 * @return size_t 
 */
inline void send(int socket, octet *msg, size_t len)
{
    size_t i = 0;
    while(i < len){
        i += send_non_blocking(socket, msg+i, len-i);
    }
}

/**
 * @brief Receive Package's Length Field via the socket.
 * 
 * @param socket socket number
 * @param a length number
 * @param len Length Field is 4 bytes
 */
inline void receive(int& socket, size_t& a, size_t len)
{
    octet blen[len];
    receive(socket, blen, len);
    a = decode_length(blen, len);
}

/**
 * @brief Receive any data
 * 
 * @param socket socket number 
 * @param msg* msg 
 * @param len msg length
 */
inline void receive(int socket, octet *msg, size_t len)
{
    size_t i = 0;
    int fail = 0;
    long wait = 1;
    while(len - i > 0){
        // receive data from socket
        int j = recv(socket, msg+i, len-i, 0);
        if (j > 0){
            i = i + j;
        }
        else if ( j < 0){
            if (errno == EAGAIN or errno == EINTR){
                if (++fail > 250000){
                    error("Unavailable too many times");
                }
                else{
                    usleep(wait *= 2);
                }
            }
            else{
                error("Receiving error - 1");
            }
        }
        else{
            throw closed_connection();
        }
    }
}

#endif