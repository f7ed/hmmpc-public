#include <iostream>
#include <pthread.h>
#include <assert.h>

#include "Networking/sockets.h"
#include "Networking/ServerSocket.h"
#include "Networking/Server.h"
#include "Tools/int.h"

/**
 * @brief Construct server through command: Server.x nmachiens PortnumBase
 * @param Number of machines connecting
 * @param Base PORTNUM address
 */
Server::Server(int argc, char **argv)
{
    if (argc != 3){
        cerr<<"Call using\n\t";
        cerr<<"Server.x n PortnumBase\n";
        cerr<<"\t\t n           = Number of machines"<<endl;
        cerr<<"\t\t PortnumBase = Base Portnum\n";
        exit(1);
    }
    // covert string to interge
    nmachines = atoi(argv[1]);
    PortnumBase = atoi(argv[2]);
}

/**
 * @brief Run a thread to start server.
 * 
 * @param server*
 * @return void* 
 */
void* Server::start_in_thread(void* server)
{
    ((Server*)server)->start();
    return 0;
}

/**
 * @brief Start the server to construct the communication network.
 * 
 */
void Server::start()
{
    int i;
    names.resize(nmachines);
    ports.resize(nmachines);

    // Inital the sockets
    socket_num.resize(nmachines);
    for (i = 0; i < nmachines; i++){
        socket_num[i] = -1;
    }

    // The Server's listening port number is one lower, which is to avoid conflict with players
    // 1. Construct a listening socket
    ServerSocket server(PortnumBase - 1);/* socket() bind() and listen() */
    // 2. Accept clients to store the conneciton socket.
    server.init();/* Accept clients: accept() and recieve the client_id to store connection socket*/
    // server.accept_clients();
    // 3. Store the socket_num from clients<client_id, connection_sockets>
    for (i = 1; i < nmachines; i++){
#ifdef DEBUG_NETWORKING
        cerr<<"Waiting for player"<<i<<endl;
#endif
        socket_num[i] = server.get_connection_socket("P"+to_string(i));
#ifdef DEBUG_NETWORKING
        cerr<<"Connected to player "<<i<<endl;
#endif
    }

    // 4. Get names from all clients
    for (i = 0; i < nmachines; i++){
        get_name(i);
    }

    // Check the setup, party 0 dosen't matter.
    // all are on local v.s. none are on local
    bool all_on_local = true, none_on_local = true;
    for (i = 1; i < nmachines; i++){
        bool on_local = string((char*)names[i]).compare("127.0.0.1");
        all_on_local &= on_local;
        none_on_local &= not on_local;
    }
    if (not all_on_local and not none_on_local){
        cout<<"You cannot address Server.x by localhost if using different hosts"<<endl;
        exit(1);
    }

    // 5. Send all names to each clients
    for (i = 0; i < nmachines; i++){
        send_names(i);
    }

    for (i = 0; i < nmachines; i++){
        delete[] names[i];
    }

    // 6. The coordination server's job is done.
    for(int i = 0; i < nmachines; i++){
        close(socket_num[i]);
    }

}

/**
 * @brief Get Name Package from the player
 * Package: [name||port]
 * Name field: 512 bytes.
 * Port field: 4 bytes.
 * @param num player_number
 */
void Server::get_name(int num)
{
#ifdef DEBUG_NETWORKING
    cerr<<"Player "<<num<<" started."<<endl;
#endif
    octet my_name[512];
    receive(socket_num[num], my_name, 512);
    receive(socket_num[num], (octet*)&ports[num], 4);
#ifdef DEBUG_NETWORKING
    cerr<<"Player "<<num<<" sent (IP for info only)" << my_name << ":"
        <<ports[num]<<endl;
#endif

    // Get client IP through client name
    get_ip(num);
}

/**
 * @brief Get client IP throw client names
 * 
 * @param num player number
 */
void Server::get_ip(int num)
{
    struct sockaddr_storage addr;
    socklen_t len = sizeof addr;

    getpeername(socket_num[num], (struct sockaddr*)&addr, &len);

    // supports both IPv4 and Ipv6
    char ipstr[INET6_ADDRSTRLEN];
    // IPv4
    if (addr.ss_family == AF_INET){
        struct sockaddr_in *s = (struct sockaddr_in *)&addr;
        inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
    }
    else{
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
        inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
    }

    names[num] = new octet[512];
    memset(names[num], 0, 512);
    strncpy((char*)names[num], ipstr, INET6_ADDRSTRLEN);

#ifdef DEBUG_NETWORKING
    cerr<<"Client IP address: "<<names[num]<<endl;
#endif
}

/**
 * @brief Server send all clients' machine names back to each client
 * @param client number
 **/

/**
 * @brief Send all Name Packages to all players.
 * Name Package: [name, port]
 * @param num player number
 */
void Server::send_names(int num)
{
    // send nmachines(4)
    send(socket_num[num], nmachines, 4);
    for (int i = 0; i < nmachines; i++){
        send(socket_num[num], names[i], 512);
        send(socket_num[num], (octet*)&ports[i], 4);
    }
}

