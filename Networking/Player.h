#ifndef _Player
#define _Player

#include <vector>
#include <set>
#include <iostream>
#include <fstream>

#include "Tools/int.h"
#include "Tools/octetStream.h"
#include "Networking/ServerSocket.h"
#include "Networking/Sender.h"
#include "Networking/Receiver.h"
#include "Networking/sockets.h"

using namespace std;

class Server;
class ServerSockt;

// Network setup including names and ports etc.
class Names
{
    friend class Player;  
    friend class PlainPlayer;

    vector<string> names;/* All players' names/ips */
    vector<int> ports;/* All players' listening ports */
    int nplayers;/* number of players */
    int portnum_base;/* base of portnum */
    int player_no;/* player numer */

    // Player as a server
    ServerSocket* server;
    void setup_server();/* Setup client's server to listen */

    int default_port(int player_no) {return portnum_base + player_no;}

    // Player as a client.
    void setup_ports();/* Set up all players' listening ports */
    void setup_names(const char *servername, int my_port);/* Set up to get all players' names/ips. */

public:

    static const int DEFAULT_PORT = -1;

    // Initialize with the coordination server.
    // BUG
    void init(int n, int playernum, int pnb, int my_ports, const char* servername);
    Names(int n, int playernum, int pnb, int my_port, const char* servername):Names()
    {init(n, playernum, pnb, my_port, servername);}

    /**
     * Initialize from file. One party per line, format ``<hostname>[:<port>]``
     * @param player my number
     * @param pnb base port number
     * @param hostsfile filename
     * @param players number of players (0 to take from file)
     */
    void init(int player, int pnb, const string& hostsfile, int players = 0);
    Names(int player, int pnb, const string& hostsfile) : Names()
    {init(player, pnb, hostsfile); }
  
    Names() : nplayers(1), portnum_base(-1), player_no(0), server(0) {}
    Names(const Names& other);
    ~Names();

    // get private value
    int num_players() const {return nplayers;}/* get nplayers */
    int my_num() const {return player_no;}/* get player number */
    const string get_name(int i)const {return names[i];}/* get the name of player i */
    const int get_port(int i)const {return ports[i];}/*get the port of player i*/
    int get_portnum_base()const{return portnum_base;}/* get the base of portnum */
};

/**
 * @brief Communication Status.
 * It's a struct, which is all public.
 */
struct CommStats
{
    size_t data, rounds;
    Timer timer;
    CommStats(): data(0), rounds(0){}

    // Timer operate
    /**
     * @brief Add communication data length
     * 
     * @param length length of communication data
     * @return Timer& 
     */
    Timer& add(size_t length){
#ifdef DEBUG_NETWORKING
        cout<<"add"<<length<<endl;
#endif
        data += length;
        rounds++;
        return timer;
    }
    /**
     * @brief Updata commnucation status (data length and rounds)
     * and return an uninitialized timer.
     * @param os 
     * @return Timer& 
     */
    Timer& add(const octetStream& os){return add(os.get_length());}
    
    void add(const octetStream& os, const TimeScope& scope)
    {add(os) += scope;}

    CommStats& operator+=(const CommStats& other);
    CommStats& operator-=(const CommStats& other);
};

/**
 * @brief map<string, CommStats>
 * - Sending to all
 * - Sending directly
 * - Receiving directly
 */
class NamedCommStats: public map<string, CommStats>
{
public:
    // legnth of data sent
    size_t sent;

    NamedCommStats():sent(0){}

    NamedCommStats& operator+=(const NamedCommStats& other);
    NamedCommStats operator+(const NamedCommStats& other) const;
    NamedCommStats operator-(const NamedCommStats& other) const;
    size_t total_data();
    size_t total_rounds();

    void print(bool newline = false);
#ifdef DEBUG_NETWORKING
    CommStats& operator[](const string& name){
        auto& res = map<string, CommStats>::operator[](name);
        cout << name << " after " << res.data << endl;
        return res;
    }
#endif
};

// Abstract class for two-/multi- player communication
class PlayerBase
{
protected:
    int player_no;/* player number */

public:
    size_t& sent;/* sent length */
    mutable Timer timer;
    mutable NamedCommStats comm_stats;

    PlayerBase(int player_no):player_no(player_no), sent(comm_stats.sent){}
    virtual ~PlayerBase();

    int my_real_num()const {return player_no;}/* get my player number */
    virtual int my_num() const = 0;/* Return my player number */
    virtual int num_players() const = 0;/* Return nplayers */
};

// Abstract class for multi-player communication.
class Player:public PlayerBase
{
protected:
    int nplayers;
    
public:
    // reference to const: cannot modify it via N
    const Names& N;/* Player's Network setup */

    Player(const Names& Nms);
    virtual ~Player();

    // Get values
    int num_players() const {return nplayers;}/* get nplayer */
    int my_num() const {return player_no;}/* get my player number */
    int get_player(int offset) const{return positive_modulo(my_num() + offset, num_players());}
    int get_offset(int other_player) const { return positive_modulo(other_player - my_num(), num_players()); }
    int get_relative(int pking)const {return positive_modulo(my_num() - pking, num_players());}
    /* The following functions generally update the statistics
     * and then call the *_no_stats equivalent is specified by subclasses*/
    
    // send to all players the o. (1 round)
    virtual void send_all(const octetStream& o) const;
    void send_all_no_stats(const octetStream& o)const;
    
    // send to a player (1 round)
    void send_to(int player, const octetStream& o)const;
    virtual void send_to_no_stats(int player, const octetStream& o) const = 0;
    // send to P[i+offset]
    void send_relative(int offset, const octetStream& o)const;/* send relatively by offset */
    // send os[i] to Pi
    void send_respective(const octetStreams& os)const;

    // Receive os[i] from Pi
    void receive_respective(octetStreams &os)const;
    // Receive from all other players. (1 round)
    void receive_all_no_stats(octetStreams& os)const;
    void receive_relative(octetStreams &os)const;

    // Receive from a specific player
    void receive_player(int i, octetStream& o)const;
    virtual void receive_player_no_stats(int i, octetStream& o) const = 0;
    void receive_relative(int offset, octetStream& o)const;

    void reset_communication();
    void print_communication();
};

class MultiPlayer: public Player
{
protected:
    vector<int> sockets;/* Connection socket to send and receive */

    // self send socket: send_to_self
    // self receive socket: sockets[my_number()]
    int send_to_self_socket;/* = 0 */
    int socket_to_send(int player)const 
    {return player == player_no ? send_to_self_socket : sockets[player];}

    int socket(int i) const {return sockets[i];}/* Return the connection sockets to player i */

public:
    MultiPlayer(const Names& Nms);
    virtual ~MultiPlayer();

    virtual void send_to_no_stats(int player, const octetStream& o)const;
    virtual void receive_player_no_stats(int i, octetStream& o) const;

    
};

/**
 * @brief Plaintext multi-playercommunication
 * 
 */
class PlainPlayer: public MultiPlayer
{
    // Set up connection sockets with other players.
    void setup_sockets(const vector<string>& names, const vector<int>& ports,
        const string& id_base, ServerSocket& server);
public:
    // Construct a new Plain Player and setup the socket network with other player.
    PlainPlayer(const Names& Nms, const string& id);
    ~PlainPlayer();
};

class ThreadPlayer: public PlainPlayer
{
public:
    mutable vector<Receiver*> receivers;/* Each thread is a Receiver to receive from a specific player */
    mutable vector<Sender*> senders;/* Each thread is a Sender to send to a specific player */
    
    // Construct a new Thread Player and run 2*nplayers threads to Receive and Send.
    ThreadPlayer(const Names& Nms, const string& id_base);
    virtual ~ThreadPlayer();

    void request_receive(int i, octetStream& o)const;/* Request to receive data from player i. */
    void wait_receive(int i, octetStream& o) const;
    void receive_player_no_stats(int i, octetStream& o) const;

    void request_send(int i, const octetStream& o)const;
    void wait_send(int i, const octetStream& o)const;
    void send_to_no_stats(int player, const octetStream& o)const;
    
    void send_all(const octetStream& o) const;
    void request_send_all(const octetStream&o)const; // request part
    void wait_send_all(const octetStream&o)const; // wait part

    void send_respective(const octetStreams &os)const;
    void request_send_respective(const octetStreams &os)const;
    void wait_send_respective(const octetStreams &os)const;

    // Send to a set of parties.
    void send_respective(int start, int nSize, const octetStreams &os)const;
    void request_send_respective(int start, int nSize, const octetStreams &os)const;
    void wait_send_respective(int start, int nSize, const octetStreams &os)const;

    void receive_respective(octetStreams &os)const;
    void request_receive_respective(octetStreams &os)const;
    void wait_receive_respective(octetStreams&os)const;
    
};


#endif