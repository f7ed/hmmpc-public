#include <sys/select.h>
#include <utility>
#include <assert.h>

#include "Tools/Exceptions.h"
#include "Tools/time-func.h"
#include "Tools/int.h"
#include "Networking/Server.h"
#include "Networking/ServerSocket.h"
#include "Networking/sockets.h"
#include "Networking/Player.h"

using namespace std;

/**
 * @brief Initialize with the coordination server.
 * @param player_no: my player number
 * @param pnb: base port number
 * @param my_port: my port number
 * @param servername: location of server
 */
void Names::init(int n, int playernum, int pnb, int my_port , const char *servername)
{
    nplayers = n;
    player_no = playernum;
    portnum_base = pnb;
    my_port = default_port(playernum);
    // 1. communication with coordination server
    setup_names(servername, my_port);
    // 2. setup a server to construct conneciton sockets with other player. 
    setup_server();
}

// initialize names from file, no Server.x coordination.
void Names::init(int player, int pnb, const string& filename, int nplayers_wanted)
{
    ifstream hostsfile(filename.c_str());
    if (hostsfile.fail()){
        stringstream ss;
        ss << "Error opening " << filename << ". See HOSTS.example for an example.";
        throw file_error(ss.str().c_str());
    }
    player_no = player;
    nplayers = 0;
    portnum_base = pnb;
    string line;
    ports.clear();
    while (getline(hostsfile, line)){
        if (line.length() > 0 && line.at(0) != '#') {
            auto pos = line.find(':');
            if (pos == string::npos){
                names.push_back(line);
                ports.push_back(default_port(nplayers));
            }
            else{
                names.push_back(line.substr(0, pos));
                int port;
                stringstream(line.substr(pos + 1)) >> port;
                ports.push_back(port);
            }   
            nplayers++;
            if (nplayers_wanted > 0 and nplayers_wanted == nplayers)
                break;
        }
    }
    // Close the file
    hostsfile.close();
    if (nplayers_wanted > 0 and nplayers_wanted != nplayers)
        throw runtime_error("not enought hosts in HOSTS");
#ifdef DEBUG_NETWORKING
    cerr << "Got list of " << nplayers << " players from file: " << endl;
    for (unsigned int i = 0; i < names.size(); i++)
        cerr << "    " << names[i] << ":" << ports[i] << endl;
#endif
    setup_server();
}

/**
 * @brief Set up all players' listening ports.
 * 
 */
void Names::setup_ports()
{
    ports.resize(nplayers);
    for (int i = 0; i < nplayers; i++){
        ports[i] = default_port(i);
    }
}

/**
 * @brief Set up to get all players' names/ips.
 * 
 * @param servername server's name(ip)
 * @param my_port client portnum to accept
 */
void Names::setup_names (const char *servername, int my_port)
{
    // Inital the listening port.
    if (my_port == DEFAULT_PORT){
        my_port = default_port(player_no);
    }

    int socket_num;
    // 1. Connect to the coordination server(servername, pn) 
    // pn: coordination server's default listening port
    int pn = portnum_base - 1;
    set_up_client_socket(socket_num, servername, pn);

#ifdef DEBUG_NETWORKING
    cerr<<"The socket num is "<<socket_num<<endl;
#endif

    // 2. Send client_id to the coordination server
    octetStream("P"+to_string(player_no)).Send(socket_num);
#ifdef DEBUG_NETWORKING
    cerr<<"Sent Client_id "<<player_no<<" to "<<servername<<":"<<pn<<endl;
#endif

    // 3. Send my name(/ip)||port to the coordination server.
    octet my_name[512];
    memset(my_name, 0, sizeof my_name);
    sockaddr_in address;
    socklen_t size = sizeof address;
    getsockname(socket_num, (sockaddr*)&address, &size);
    // convert network address to ip string
    // Now default ip is 127.0.0.1
    char* name = inet_ntoa(address.sin_addr);
    // char* name = "127.0.0.1";
    // 16: max length of IP address with ending 0
    strncpy((char*)my_name, name, 16);
    send(socket_num, my_name, 512);
    send(socket_num, (octet*)&my_port, 4);
    /* send my_port(int-4) as octetStream */
#ifdef DEBUG_NETWORKING
    cerr<<"Send my name and my port."<<endl;
    fprintf(stderr, "My name(ip) = %s\n", my_name);
    cerr<<"My port = "<<my_port<<endl;
    cerr<<"My number = "<<player_no<<endl;
#endif

    // 4. Now get the set of names from the server
    int i;
    size_t tmp;
    // Receive length field
    receive(socket_num, tmp, 4);
    nplayers = tmp;
#ifdef DEBUG_NETWORKING
    cerr<<nplayers<<" Players\n";
#endif
    names.resize(nplayers);
    ports.resize(nplayers);
    // receive name and port
    for (i = 0; i < nplayers; i ++){
        octet tmp[512];
        receive(socket_num, tmp, 512);
        names[i] = (char*)tmp;
        receive(socket_num, (octet*)&ports[i], 4);
#ifdef DEBUG_NETWORKING
        cerr<<"Player "<<i<<" is running on machine "<<names[i]<<endl;
        cerr<<"Player "<<i<<"'s Portnum is "<<ports[i]<<endl;
#endif
    }
    close_client_socket(socket_num);
}

/**
 * @brief Setup client's server to listen
 * 
 */
void Names::setup_server()
{
    server = new ServerSocket(ports[player_no]);
#ifdef DEBUG_NETWORKING
    cerr<<"New a serversocket on port ports:"<<ports[player_no]<<endl;
#endif
    server->init();
}


Names::~Names()
{
    if (server!=0){
        delete server;
    }
}

CommStats& CommStats::operator +=(const CommStats& other)
{
  data += other.data;
  rounds += other.rounds;
  timer += other.timer;
  return *this;
}

NamedCommStats& NamedCommStats::operator +=(const NamedCommStats& other)
{
  sent += other.sent;
  for (auto it = other.begin(); it != other.end(); it++)
    (*this)[it->first] += it->second;
  return *this;
}

NamedCommStats NamedCommStats::operator +(const NamedCommStats& other) const
{
    auto res = *this;
    res += other;
    return res;
}

CommStats& CommStats::operator -=(const CommStats& other)
{
    data -= other.data;
    rounds -= other.rounds;
    timer -= other.timer;
    return *this;
}

NamedCommStats NamedCommStats::operator -(const NamedCommStats& other) const
{
    NamedCommStats res = *this;
    res.sent = sent - other.sent;
    for (auto it = other.begin(); it != other.end(); it++){
        res[it->first] -= it->second;
    }
    return res;
}

size_t NamedCommStats::total_data()
{
    size_t res = 0;
    for (auto& x : *this){
        res += x.second.data;
    }
    return res;
}

size_t NamedCommStats::total_rounds()
{
    size_t res = 0;
    for(auto& x:*this){
        res+=x.second.rounds;
    }
    return res;
}

void NamedCommStats::print(bool newline)
{
    for (auto it = begin(); it != end(); it++)
    {
        if (it->second.data){
            cerr<< it->first << " " << 1e-6 * it->second.data << " MB in "
                << it->second.rounds << " rounds, taking " << it->second.timer.elapsed()
                << " seconds" << endl;
        }
      
    }
    if (size() and newline){
        cerr << endl;
    } 
}

/**
 * @brief Construct a new Player:: Player object
 * 
 * @param Nms Use name(Nms) to inital the Player.
 * (Nms is a reference, and N is a reference to const)
 * Although member vars(nplayers...) are private, 
 * class Player is a friend to Names.
 */
Player::Player(const Names& Nms):
    PlayerBase(Nms.my_num()), N(Nms)
{
    nplayers = Nms.nplayers;
    player_no = Nms.player_no;
}

Player::~Player()
{

}

PlayerBase::~PlayerBase()
{
    
}

void Player::reset_communication()
{
    timer.reset();
    timer.stop();
    sent = 0;
    comm_stats.clear();
}

void Player::print_communication()
{
    cerr << "Time = "<<timer.elapsed()<<" seconds"<<endl;
    cerr << "Data sent = "<< sent/1e6 <<" MB in ~"<<comm_stats.total_rounds()<<" rounds"<<endl;
    octetStream o;
    o.store(sent);
    send_all_no_stats(o);
    octetStreams os;
    receive_all_no_stats(os);
    size_t global = sent;
    for(int i = 0; i < nplayers; i++)
        if(i != player_no)
            global += os[i].get_int(8);
    cerr << "Global data sent = "<<global/1e6<<" MB (all parties)"<<endl;
    cout<<endl;
}

/**
 * @brief Construct a new Multi Player:: Multi Player object
 * (overrides Player)
 * @param Nms 
 */
MultiPlayer::MultiPlayer(const Names& Nms):
    Player(Nms), send_to_self_socket(0)
{
    sockets.resize(Nms.num_players());
}

MultiPlayer::~MultiPlayer()
{

}

/**
 * @brief Construct a new Plain Player and setup the socket network with other player.
 * 
 * @param Nms 
 * @param id 
 */
PlainPlayer::PlainPlayer(const Names& Nms, const string& id):
    MultiPlayer(Nms)
{
    if (Nms.num_players() > 1){
        setup_sockets(Nms.names, Nms.ports, id, *Nms.server);
    }
}
PlainPlayer::~PlainPlayer()
{
    if(num_players()>1){
        for(auto socket:sockets){
            close_client_socket(socket);
        }
        close_client_socket(send_to_self_socket);
    }
}

/**
 * @brief Construct a new Thread Player and run 2*nplayers threads to Receive and Send.
 * There are n threads runing the Receiver, and the other runing the Sender.
 * 
 * @param Nms Network setup
 * @param id_base 
 */
ThreadPlayer::ThreadPlayer(const Names& Nms, const string& id_base):
    PlainPlayer(Nms, id_base)
{
    for (int i = 0; i < Nms.num_players(); i++)
    {
        receivers.push_back(new Receiver(sockets[i]));
        senders.push_back(new Sender(socket_to_send(i)));
    }
}

ThreadPlayer::~ThreadPlayer()
{
    for (unsigned int i = 0; i < receivers.size(); i++){
#ifdef VERBOSE
        if(receivers[i]->timer.elapsed()>0){
            cerr<<"Waiting for receiving from "<<i<<": "<<receivers[i]->timer.elapsed()<<endl;
        }
#endif
        delete receivers[i];
    }
    for (unsigned int i = 0; i < senders.size(); i++){
#ifdef VERBOSE
        if(senders[i]->timer.elapsed()>0){
            cerr<<"Waiting for sending from "<<i<<": "<<senders[i]->timer.elapsed()<<endl;
        }
#endif
        delete senders[i];
    }
}

/**
 * @brief Send to all players
 * 
 * @param o 
 */
void Player::send_all(const octetStream& o)const
{
#ifdef DEBUG_NETWORKING
    cerr<<"sending to all players"<<endl;
#endif
    // CommStats.add(o) return an uninitalized timer,
    // which TimeScope(timer) use to timer.start()
    TimeScope ts(comm_stats["Sending to all"].add(o));
    for (int i = 0; i < nplayers; i++){
        if(i != player_no){
            send_to_no_stats(i, o);
        }
    }
    sent += o.get_length() * (num_players() - 1);
}

void Player::send_all_no_stats(const octetStream& o)const
{
    for(int i = 0; i < nplayers; i++){
        if(i != player_no){
            send_to_no_stats(i, o);
        }
    }
}

/**
 * @brief Send os[i] to Pi respectively.
 * 
 * @param os 
 */
void Player::send_respective(const octetStreams & os)const
{
    assert(os.size()==nplayers);
    for (int i = 0; i < nplayers; i++){
        if(i != player_no){
            send_to(i, os[i]);
        }
    }
}

void Player::receive_respective(octetStreams &os)const
{
    assert(os.size()==nplayers);
    for (int i = 0; i < nplayers; i++){
        if(i != player_no){
            receive_player(i, os[i]);
        }
    }
}

/**
 * @brief Send to the player
 * 
 * @param player player number
 * @param o 
 */
void Player::send_to(int player, const octetStream& o)const
{
#ifdef DEBUG_NETWORKING
    cerr<<"sending to "<<player<<endl;
#endif
    TimeScope ts(comm_stats["Sending directly"].add(o));
    send_to_no_stats(player, o);
    sent += o.get_length();
}

/**
 * @brief send relatively by offset
 * 
 * @param offset 
 * @param o 
 */
void Player::send_relative(int offset, const octetStream& o)const
{
    send_to(positive_modulo(my_num()+offset, num_players()), o);
}

/**
 * @brief Receive from the relative offset player
 * 
 * @param offset 
 * @param o 
 */
void Player::receive_relative(int offset, octetStream& o)const
{
    receive_player(positive_modulo(my_num()+offset, num_players()), o);
}

/**
 * @brief Receive stream from relatively next t parties.
 * (os[i] from Pi)
 * 
 * @param os size=t+1
 */
void Player::receive_relative(octetStreams &os)const
{
    int len = os.size();
    // Receive from the next party
    for(int offset = 1; offset < len; offset++){
        receive_player(offset, os[offset]);
    }
}

/**
 * @brief [Multiparty]Sent to a player
 * 
 * @param player player number
 * @param o send data
 */
void MultiPlayer::send_to_no_stats(int player, const octetStream& o) const
{
    int socket = socket_to_send(player);
    o.Send(socket);
}


void Player::receive_all_no_stats(octetStreams &os)const
{
    os.resize(num_players());
    for(int i = 0; i < nplayers; i++){
        if(i != player_no){
            receive_player_no_stats(i, os[i]);
        }
    }
}

/**
 * @brief Receive from a specific player
 * 
 * @param i 
 * @param o 
 */
void Player::receive_player(int i, octetStream& o) const 
{
#ifdef DEBUG_NETWORKING
    cerr<<"Receiving from "<<i<<endl;
#endif
    // TimeScope ts(comm_stats["Receiving directly"].add(o));
    receive_player_no_stats(i, o);
}

/**
 * @brief [Multiparty]Receive from a specific player
 * 
 * @param i 
 * @param o 
 */
void MultiPlayer::receive_player_no_stats(int i, octetStream& o) const 
{
    // reset length header and write header before Receving
    o.reset_write_head();
    o.Receive(sockets[i]);
}

/**
 * @brief Set up connection sockets with other players.
 * Between the player A and the player B (#A < #B),
 * A is the client while B is the server.
 * As a client: connect to my_num() --- num_players().
 * As a server: listen from the 0 --- my_num().
 * 
 * @param names all names
 * @param ports all ports
 * @param id_base 
 * @param server player's server
 */
void PlainPlayer::setup_sockets (const vector<string>& names,
    const vector<int>& ports, const string& id_base, ServerSocket& server)
{
    sockets.resize(nplayers);
    
    // 1. Set up the client side: client connect the server to get socket
    for (int i = player_no; i < nplayers; i++){
        auto pn = id_base+"P"+to_string(player_no);
        // send to self socket
        if(i == player_no){
            const char* localhost = "127.0.0.1";
#ifdef DEBUG_NETWORKING
            fprintf(stderr, "Setting up send to self socket to %s:%d with id %s\n",
                localhost, ports[i], pn.c_str());
#endif
            set_up_client_socket(sockets[i], localhost, ports[i]);
        }
        // send to others
        else{
#ifdef DEBUG_NETWORKING
            fprintf(stderr, "Setting up client to %s:%d with id %s\n",
                names[i].c_str(), ports[i], pn.c_str());
#endif
            set_up_client_socket(sockets[i], names[i].c_str(), ports[i]);
        }
        // 2. Send client id to the player's server
        // The player's server will receive it to get socket
        octetStream(pn).Send(sockets[i]);
    }

    // send_to_self_socket
    send_to_self_socket = sockets[player_no];

    // 3. Setting up the server side to get connection socket
    for (int i = 0; i <= player_no; i++){
        auto id = id_base+"P"+to_string(i);
#ifdef DEBUG_NETWORKING
        fprintf(stderr, "As a server, waiting for client with id %s to connect.\n",
            id.c_str());
#endif
        sockets[i] = server.get_connection_socket(id);
    }
    // sockets[player_no]: receive from self socket

    for (int i = 0; i < nplayers; i++){
        // timeout of 5 minutes
        struct timeval tv;
        tv.tv_sec = 300;
        tv.tv_usec = 0;
        int fl = setsockopt(sockets[i], SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));
        if (fl < 0){
            error("set_up_socket:setsocketopt");
        }
    }
}

/**
 * @brief Request to receive data from player i.
 * 
 * @param i 
 * @param o 
 */
void ThreadPlayer::request_receive(int i, octetStream& o)const
{
    receivers[i]->request(o);
}

void ThreadPlayer::wait_receive(int i, octetStream& o)const
{
    receivers[i]->wait(o);
}

/**
 * @brief Request to receive data from player i.
 * 
 * @param i 
 * @param o 
 */
void ThreadPlayer::receive_player_no_stats(int i, octetStream& o) const
{
    request_receive(i, o);
    wait_receive(i, o);
}

void ThreadPlayer::request_send(int i, const octetStream& o)const
{
    senders[i]->request(o);
}

void ThreadPlayer::wait_send(int i, const octetStream& o)const
{
    senders[i]->wait(o);
}

void ThreadPlayer::send_to_no_stats(int player, const octetStream& o) const
{
    request_send(player, o);
    wait_send(player, o);
}

void ThreadPlayer::send_all(const octetStream& o)const
{
    TimeScope ts(comm_stats["Sending to all"].add(o));
    for(int i = 0; i < nplayers; i++){
        if(i!=player_no){
            senders[i]->request(o);
        }
    }
    for(int i = 0; i < nplayers; i++){
        if(i!=player_no){
            senders[i]->wait(o);
        }
    }
    sent += o.get_length() * (num_players() - 1);
}

void ThreadPlayer::request_send_all(const octetStream &o)const
{
    TimeScope ts(comm_stats["Sending to all"].add(o));
    for(int i = 0; i < nplayers; i++){
        if(i!=player_no){
            senders[i]->request(o);
        }
    }
    sent += o.get_length() * (num_players() - 1);
}

void ThreadPlayer::wait_send_all(const octetStream &o)const
{
    for(int i = 0; i < nplayers; i++){
        if(i!=player_no){
            senders[i]->wait(o);
        }
    }
}


void ThreadPlayer::send_respective(const octetStreams& os)const
{
    size_t length = 0;
    for(int i = 0; i < nplayers; i++){
        if(i!=player_no){
            senders[i]->request(os[i]);
            length += os[i].get_length();
        }
    }
    for(int i = 0; i < nplayers; i++){
        if(i!=player_no){
            senders[i]->wait(os[i]);
        }
    }
    TimeScope ts(comm_stats["Sending to all parties respectively"].add(length));
    sent += length;
}

void ThreadPlayer::request_send_respective(const octetStreams&os)const
{
    size_t length = 0;
    for(int i = 0; i < nplayers; i++){
        if(i!=player_no){
            senders[i]->request(os[i]);
            length += os[i].get_length();
        }
    }
    TimeScope ts(comm_stats["Sending to all parties respectively"].add(length));
    sent += length;
}

void ThreadPlayer::wait_send_respective(const octetStreams&os)const
{
    for(int i = 0; i < nplayers; i++){
        if(i!=player_no){
            senders[i]->wait(os[i]);
        }
    }
}

void ThreadPlayer::receive_respective(octetStreams &os)const
{
    for(int i = 0; i < nplayers; i++){
        if(i!=player_no){
            receivers[i]->request(os[i]);
        }
    }
    for(int i = 0; i < nplayers; i++){
        if(i!=player_no){
            receivers[i]->wait(os[i]);
        }
    }
}

void ThreadPlayer::request_receive_respective(octetStreams &os)const
{
    for(int i = 0; i < nplayers; i++){
        if(i!=player_no){
            receivers[i]->request(os[i]);
        }
    }
}

void ThreadPlayer::wait_receive_respective(octetStreams &os)const
{
    for(int i = 0; i < nplayers; i++){
        if(i!=player_no){
            receivers[i]->wait(os[i]);
        }
    }
}

// Send a set of parties, starting from 'start' with size nSize.
void ThreadPlayer::send_respective(int start, int nSize, const octetStreams &os)const
{
    assert(nSize==os.size());
    size_t length = 0;
    for(int i = 0; i < nSize; i++){
        int id = positive_modulo(start+i, num_players());
        if(id!=player_no){
            senders[id]->request(os[i]);
            length += os[i].get_length();
        }
    }

    for(int i = 0; i < nSize; i++){
        int id = positive_modulo(start+i, num_players());
        if(id!=player_no){
            senders[id]->wait(os[i]);
        }
    }
    TimeScope ts(comm_stats["Sending to a set of parties respectively"].add(length));
    sent += length;
}

void ThreadPlayer::request_send_respective(int start, int nSize, const octetStreams &os)const
{
    assert(nSize==os.size());
    size_t length = 0;
    for(int i = 0; i < nSize; i++){
        int id = positive_modulo(start+i, num_players());
        if(id!=player_no){
            senders[id]->request(os[i]);
            length += os[i].get_length();
        }
    }
    TimeScope ts(comm_stats["Sending to a set of parties respectively"].add(length));
    sent += length;
}

void ThreadPlayer::wait_send_respective(int start, int nSize, const octetStreams &os)const
{
    for(int i = 0; i < nSize; i++){
        int id = positive_modulo(start+i, num_players());
        if(id!=player_no){
            senders[id]->wait(os[i]);
        }
    }
}
