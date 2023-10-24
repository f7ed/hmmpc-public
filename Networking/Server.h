#ifndef NETWORKING_SERVER_H_
#define NETWORKING_SERVER_H_

#include <vector>

#include "Networking/data.h"
#include "Networking/Player.h"
using namespace std;

// The coordination server
class Server
{
    vector<octet*> names;/* eventually store clients' ip/name */
    vector<int> ports;/* Players' listening ports */
    vector<int>socket_num;/* Sockets between the coordination server and the player */
    int nmachines;/* number of machines/players */
    int PortnumBase;

public:
    Server(int arg, char** argv);/* Construct server through command: Server.x nmachiens PortnumBase */
    Server(int nmachines, int PortnumBase):nmachines(nmachines), PortnumBase(PortnumBase){}
    
    void start();/* Start the server to construct the communication network. */
    static void* start_in_thread(void* server);/* Run a thread to start server. */

    void get_ip(int num);/* Get client IP throw client names */
    void get_name(int num);/* Get Name Package from the player */
    void send_names(int num);/* Send all Name Packages to all players. */
};
#endif