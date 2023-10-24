#ifndef NETWORKING_SERVERSOCKET_H_
#define NETWORKING_SERVERSOCKET_H_

#include <map>
#include <set>
#include <queue>
#include <string>
#include <pthread.h>
#include <netinet/tcp.h>

#include "Tools/Signal.h"

using namespace std;

class ServerJob;

class ServerSocket
{
protected:
    int main_socket, portnum;
    map<string,int> clients;/* <client_id, the connection sockets> */
    set<string> used;
    Signal data_signal;
    pthread_t thread;
    vector<ServerJob*>jobs;

    // put in proctected so that disable copying
    ServerSocket(const ServerSocket& other);
    
    void process_connection(int socket, const string& client_id);

    // virtual function which can be overrided by AnonymousSocket
    virtual void process_client(const string&) {}

public:
    ServerSocket(int Portname);/* socket(); bind(); listen(); */
    ServerSocket(int Portname, const char* ip);/* socket(); bind(); listen(); */
    virtual ~ServerSocket();/* close() */

    virtual void init();/* creat a thread to execute the fucntion */
    
    virtual void accept_clients(); /* accept(); rcve() client id; */
    // create a ServerJob(thread) to wait for the client
    void wait_for_client_id(int socket, struct sockaddr dest);

    int get_connection_socket(const string& id);/* Retrive the connection socket from client<client_id, socket> */
};

// class AnonymousServerSocket: public ServerSocket
// {
// private:
//     // No. of accepted connections in this instance
//     // 之前已经接受过的连接，现在关闭了
//     queue<string>client_connection_queue;
//     void process_client(const string& client_id);

// public:
//     AnonymousServerSocket(int Portnum): ServerSocket(Portnum){};
// };
#endif