#include <iostream>

#include "Networking/Receiver.h"

using namespace std;

/**
 * @brief Construct a new Receiver to receive data via the socket.
 * A Receiver is binded to a specific Player.
 * 
 * @param socket socket number
 */
Receiver::Receiver(int socket): socket(socket), thread(0)
{
    start();
}

Receiver::~Receiver()
{
    stop();
}

/**
 * @brief Create a thread to execute run()(receive data).
 * 
 */
void Receiver::start()
{
    pthread_create(&thread, 0, run_thread, this);
}

void* Receiver::run_thread(void* receiver)
{
    ((Receiver*)receiver)->run();
    return 0;
}

/**
 * @brief Receive data in the thread.
 * in queue: to recieve
 * out queue: received
 * 
 */
void Receiver::run()
{
    octetStream* os = 0;
    while(in.pop(os)){
        os->reset_write_head();
#ifdef VERBOSE
        timer.start();
#endif
        os->Receive(socket);
#ifdef VERBOSE       
        timer.stop();
#endif
        out.push(os);
    }
}

void Receiver::stop()
{
    in.stop();
    pthread_join(thread, 0);
}

/**
 * @brief Request a octetStream to receive 
 * 
 * @param os 
 */
void Receiver::request(octetStream& os)
{
    in.push(&os);
}

//TODO 确保收一个，读一个？
void Receiver::wait(octetStream& os)
{
    octetStream* queued = 0;
    out.pop(queued);
    if (queued != &os){
        throw not_implemented();
    }
}