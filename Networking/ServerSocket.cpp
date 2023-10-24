#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>


#include "Networking/ServerSocket.h"
#include "Networking/sockets.h"
#include "Tools/Exceptions.h"
#include "Tools/time-func.h"
#include "Tools/int.h"
#include "Tools/octetStream.h"

using namespace std;

class ServerJob
{
    ServerSocket& server;
    int socket;
    struct sockaddr dest;

public: 
    pthread_t thread;
    ServerJob(ServerSocket& server, int socket, struct sockaddr dest):
        server(server), socket(socket), dest(dest), thread(0) {}
    
    static void* run(void* job)
    {
        // job is a pointer to ServerJob
        // *job is a ServerJob instance
        auto& server_job = *(ServerJob*)(job);
        server_job.server.wait_for_client_id(server_job.socket, server_job.dest);
        delete &server_job;
        // detach the thread to recycle resource
        pthread_detach(pthread_self());
        return 0;
    }
};

void* accept_thread(void* server_socket)
{
    ((ServerSocket*)server_socket)->accept_clients();
    return 0;
}

/**
 * @brief The server creates, binds, and listens via Portnum
 * 
 * @param Portnum 
 */
ServerSocket::ServerSocket(int Portnum): portnum(Portnum), thread(0)
{
    struct sockaddr_in serv; /* socket info about our server */

    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET; /* 设置协议族 */
    serv.sin_addr.s_addr = INADDR_ANY; /* 设置IP */
    serv.sin_port = htons(Portnum);/* 将主机上的u_short转换为网络字节流（大端序） */

    // 1. create a socket
    main_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (main_socket < 0){
        error("set_up_socket: socket");
    }

    int one = 1;
    // set socket option
    int fl=setsockopt(main_socket,SOL_SOCKET,SO_REUSEADDR,(char*)&one,sizeof(int));
    if (fl < 0){
        error("set_up_socket: setsockopt");
    }
    fl= setsockopt(main_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&one,sizeof(int));
    if (fl<0){
        error("set_up_socket:setsockopt");
    }

    octet my_name[512];
    memset(my_name, 0, sizeof(my_name));
    gethostname((char*)my_name, 512);

    // 2. bind a local address with a socket
    fl = 1;
    RunningTimer timer;
    while (fl!=0 and timer.elapsed() < 600){
        fl = ::bind(main_socket, (struct sockaddr*)&serv, sizeof(struct sockaddr));
        if (fl != 0 ){
            cerr<<"Binding to socket on "<<my_name<<":"<<Portnum
                <<" failed ("<<strerror(errno)
                <<"), trying agian in a sencond ..."<<endl;
            sleep(1);
        }
#ifdef DEBUG_NETWORKING
        else{
            cerr<<"ServerSocket is bound on port"<<Portnum<<endl;
        }
#endif
    }

    if (fl < 0){
        error("set_up_socket:bind");
    }

    // 3. start listening, allowing a queue of up to 1000 pending connection
    fl = listen(main_socket, 1000);
    if (fl < 0){
        error("set_up_socket: listen");
    }
}

ServerSocket::ServerSocket(int Portnum, const char *ip): portnum(Portnum), thread(0)
{
    struct sockaddr_in serv; /* socket info about our server */

    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET; /* 设置协议族 */
    serv.sin_addr.s_addr = inet_addr(ip); /* 设置IP */
    serv.sin_port = htons(Portnum);/* 将主机上的u_short转换为网络字节流（大端序） */

    // 1. create a socket
    main_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (main_socket < 0){
        error("set_up_socket: socket");
    }

    int one = 1;
    // set socket option
    int fl=setsockopt(main_socket,SOL_SOCKET,SO_REUSEADDR,(char*)&one,sizeof(int));
    if (fl < 0){
        error("set_up_socket: setsockopt");
    }
    fl= setsockopt(main_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&one,sizeof(int));
    if (fl<0){
        error("set_up_socket:setsockopt");
    }

    octet my_name[512];
    memset(my_name, 0, sizeof(my_name));
    gethostname((char*)my_name, 512);

    // 2. bind a local address with a socket
    fl = 1;
    RunningTimer timer;
    while (fl!=0 and timer.elapsed() < 600){
        fl = ::bind(main_socket, (struct sockaddr*)&serv, sizeof(struct sockaddr));
        if (fl != 0 ){
            cerr<<"Binding to socket on "<<my_name<<":"<<Portnum
                <<" failed ("<<strerror(errno)
                <<"), trying agian in a sencond ..."<<endl;
            sleep(1);
        }
#ifdef DEBUG_NETWORKING
        else{
            cerr<<"ServerSocket is bound on port"<<Portnum<<endl;
        }
#endif
    }

    if (fl < 0){
        error("set_up_socket:bind");
    }

    // 3. start listening, allowing a queue of up to 1000 pending connection
    fl = listen(main_socket, 1000);
    if (fl < 0){
        error("set_up_socket: listen");
    }
}

ServerSocket::~ServerSocket()
{
    pthread_cancel(thread);
    pthread_join(thread, 0);
    if (close(main_socket)){
        error("close(main_socket)");
    }
}

/**
 * @brief The server create a thread to start the server, 
 * which is ready to accept the clients.
 * 
 */
void ServerSocket::init()
{
    // create a new thread
    // the thread is created executing accept_thread with arg this
    // this is a pointer to ServerSocket
    pthread_create(&thread, 0, accept_thread, this);
}

/**
 * @brief 
 * 
 */
void ServerSocket::accept_clients()
{
    while(true){
        struct sockaddr dest;
        memset(&dest, 0, sizeof(dest));
        int socksize = sizeof(dest);
#ifdef DEBUG_NETWORKING
        fprintf(stderr, "Accepting...\n");
#endif
        // 1. Accetp the connection
        // consocket: connection socket between the coordination server and the player
        int consocket = accept(main_socket, (struct sockaddr*)&dest, (socklen_t*)&socksize);
#ifdef DEBUG_NETWORKING
        cerr<<"The connection socket is "<<consocket<<endl;
#endif     
        if (consocket < 0){
            error("set_up_socket: accept");
        }

        // 2. Receive client_id from the player.
        octetStream client_id;
        char buf[1];

        // MSG_PEEK: only copy msg from tcp buffer to buf, but not remove it from tcp buffer
        if (recv(consocket, buf, 1, MSG_PEEK|MSG_DONTWAIT) > 0){
            // Trully receive cleint_id
            client_id.Receive(consocket);
#ifdef DEBUG_NETWORKING
            cerr<<"Receving client id "<<client_id.str()<<endl;
#endif
            // Store the client connection socket.
            process_connection(consocket, client_id.str());
        }
        else{
#ifdef DEBUG_NETWORKING
            auto& conn = *(sockaddr_in*)&dest;
            //  inet_ntoa: converts an (Ipv4) Internet network address into an ASCII string
            fprintf(stderr, "deferring client on %s:%d to thread\n",
                    inet_ntoa(conn.sin_addr), ntohs(conn.sin_port));
#endif  
            // create a ServerJob(thread) to wait for the client 
            // *this: dereference the pointer = the ServerSocket instance
            // job is a pointer to ServerJob
            auto job = (new ServerJob(*this, consocket, dest));
            pthread_create(&(job->thread), 0, ServerJob::run, job);
        }  

#ifdef __APPLE__
        int flags = fcntl(consocket, F_GETFL, 0);
        int fl = fcntl(consocket, F_SETFL, O_NONBLOCK | flags);
        if (fl < 0){
            error("set non-blocking");
        }
#endif
    }
}

// create a ServerJob(thread) to wait for the client
void ServerSocket::wait_for_client_id(int socket, struct sockaddr dest)
{
    (void)dest;
    try{
        octetStream client_id;
        client_id.Receive(socket);
        process_connection(socket, client_id.str());
    }
    catch(closed_connection&){
#ifdef DEBUG_NETWORKING
        auto& conn = *(sockaddr_in*)&dest;
        fprintf(stderr, "client on %s:%d left without identification\n",
            inet_ntoa(conn.sin_addr), ntohs(conn.sin_port));
#endif
    }
}

/**
 * @brief Store the connection socket between the client and the server.
 * 
 * @param consocket connection socket
 * @param client_id id_base+"P"+to_string(player_no)
 */
void ServerSocket::process_connection(int consocket, const string& client_id)
{
    data_signal.lock();
#ifdef DEBUG_NETWORKING
    cerr<<"client"<<hex<<client_id<<" is on socket"<<dec<<consocket<<endl;
#endif
    process_client(client_id);/* Overrided by AnonymousServerSocket */
    clients[client_id] = consocket;/* Store the connection socket */
    data_signal.broadcast();/* Broadcast other threads waiting the signal */
    data_signal.unlock();
}

/**
 * @brief Retrive the connection socket from client<client_id, socket>
 * 
 * @param id client_id
 * @return int connection socket
 */
int ServerSocket::get_connection_socket(const string& id)
{
    data_signal.lock();
    // The player is connected
    if (used.find(id)!=used.end()){
        stringstream ss;
        ss<<"Connection id"<<hex<<id<<" already used";
        throw IO_Error(ss.str());
    }

    // The client_id is not stored
    while (clients.find(id) == clients.end()){
        // DEBUG
        if(data_signal.wait(60*5) == ETIMEDOUT){
            throw runtime_error("No clients after one minute");
        }
    }

    int client_socket = clients[id];
    used.insert(id);
    data_signal.unlock();
    return client_socket;
}
// void AnonymousServerSocket::process_client(const string& client_id)
// {
//     if(clients.find(client_id)!=clients.end()){
//         // 之前已经连接过这个客户，关掉之前的连接，更新为当前的连接。
//         close_client_socket(clients[client_id]);
//         client_connection_queue.push(client_id);
//     }
// }
