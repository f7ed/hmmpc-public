#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include "Networking/sockets.h"
#include "Tools/Exceptions.h"
#include "Tools/time-func.h"

using namespace std;

void error(const char *str)
{
    int old_errno = errno; /* number of last error */
    char err[1000];
    gethostname(err,1000);
    strcat(err," : ");
    strcat(err, str);
    throw runtime_error(string() + err + " : " + strerror(old_errno));
}

/**
 * @brief Client connects to the server(hostname&Portnum) to get connection socket
 * 
 * @param mysocket [out]connection socket between the client to the server
 * @param hostname [in]server's hostname
 * @param Portnum [in]server's portnum
 * @note Ther "server" can be the coordination server or a specific player.
 */
void set_up_client_socket(int& mysocket, const char *hostname, int Portnum)
{
    struct addrinfo hints, *ai = NULL, *rp;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_CANONNAME;

    // my_name: The client's hostname
    char my_name[512];
    memset(my_name, 0, sizeof(my_name));
    // 1. Retrieve the standard host name for the local computer
    gethostname((char*)my_name, 512);
#ifdef DEBUG_NETWORKING
    cerr<<"My hostname is: "<<my_name<<endl;
#endif
    // 2. Get the server's sockaddr struct
    // Debug
    //TODO to understand
    int erp;
    for (int i = 0; i < 60; i++){
        // ai: get server's info via server's hostname
        erp = getaddrinfo(hostname, NULL, &hints, &ai);
        if (erp == 0){
            break;
        }
        else{
            // gai_strerror: error info from geraddinfo
            cerr<<"getaddrinfo on "<<my_name<<" has returned '"
            <<gai_strerror(erp)<<"' for "<<hostname<<", trying again in a second..."<<endl;
            if (ai){
                freeaddrinfo(ai);
            }
            sleep(1);
        }
    }
    if(erp!=0){
        error("set_up_socket: getaddrinfo");
    }

    bool success = false;
    socklen_t len = 0;
    const struct sockaddr* addr = 0;
    for (rp=ai; rp!=NULL; rp = rp->ai_next){
        addr = ai->ai_addr;
        if (ai->ai_family == AF_INET){
            len = ai->ai_addrlen;
            success = true;
            continue;
        }
    }

    if (not success){
        for (rp = ai; rp != NULL; rp = rp->ai_next){
            cerr<<"Family on offer: "<< ai->ai_family<<endl;
        }
        runtime_error(string("No AF_INET for ")+(char*)hostname+" on "+(char*)my_name);
    }

    Timer timer;
    timer.start();
    struct sockaddr_in* addr4 = (sockaddr_in*) addr;
    addr4->sin_port = htons(Portnum);
#ifdef DEBUG_NETWORKING
    cout<<"connect to ip "<<hex<<addr4->sin_addr.s_addr
        <<" port "<<addr4->sin_port<<dec<<endl;
#endif

    int attempts = 0;
    long wait = 1;
    int fl;
    int connect_errno;
    do{
        // 3. Create the connection socket.
        // [out] mysocket = socket()
        // the connection socket between client to the server/ the other player
        mysocket = socket(AF_INET, SOCK_STREAM, 0);
        if (mysocket < 0){
            error("set_up_socket: socket");
        }
        // 3. Connect to server 
        fl = connect(mysocket, addr, len);
        connect_errno = errno;
        attempts++;
        if(fl!=0){
            close(mysocket);
            usleep(wait*=2);
#ifdef DEBUG_NETWORKING
            string msg = "Connecting to "+string(hostname)+":"
                +to_string(Portnum)+"failed";
            errno = connect_errno;
            // prints an error message to stderr
            perror(msg.c_str());
#endif
        }
        errno = connect_errno;
    }
    while(fl==-1 && (errno==ECONNREFUSED||errno==ETIMEDOUT||errno==EINPROGRESS)
        && timer.elapsed()<60);
    
    if (fl < 0){
        throw runtime_error(
            string()+"cannot connect from "+my_name+" to "+hostname
            +":"+to_string(Portnum)+" after "+to_string(attempts)
            +" attempts in one minute because "+strerror(connect_errno)+"."
        );
    }

    freeaddrinfo(ai);

    // disable Nagle's algorithm
    int one = 1;
    fl = setsockopt(mysocket, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof(int));
    if (fl < 0){
        error("set_up_socket:setsocketopt");
    }
#ifdef __APPLE__
    int flags = fcntl(mysocket, F_GETFL, 0);
    fl = fcntl(mysocket, F_SETFL, O_NONBLOCK|flags);
    if (fl < 0){
        error("set non-blocking");
    }
#endif
}

/**
 * @brief Client closes the connection sockets.
 * 
 * @param socket 
 */
void close_client_socket(int socket)
{
    if (close(socket)){
        char tmp[1000];
        sprintf(tmp, "close(%d)", socket);
        error(tmp);
    }
}