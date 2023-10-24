#include "Networking/Sender.h"

/**
 * @brief Construct a new Sender to send data via the socket.
 * Each Sender is binded to another player.
 * 
 * @param socket socket number
 */
Sender::Sender(int socket):socket(socket), thread(0)
{
    start();
}

Sender::~Sender()
{
    stop();
}

/**
 * @brief Create a thread to execute run()(send data).
 * 
 */
void Sender::start ()
{
    pthread_create(&thread, 0, run_thread, this);
}

void* Sender::run_thread (void* sender)
{
    ((Sender*)sender)->run();
    return 0;
}

/**
 * @brief Sends all octetStream in "in queue"(to send) in the thread.
 * in queue: to send
 * out queue: sent
 *
 */
void Sender::run()
{
    // pointer to const
    const octetStream* os = 0;
    while(in.pop(os))
    {
#ifdef VERBOSE
        timer.start();
#endif
        os->Send(socket);
#ifdef VERBOSE
        timer.stop();
#endif
        out.push(os);
    }
}

void Sender::stop()
{
    in.stop();
    pthread_join(thread, 0);
}

/**
 * @brief Request a octetStream to send.
 * 
 * @param os 
 */
void Sender::request(const octetStream& os)
{
    in.push(&os);
}

//TODO don't understand
void Sender::wait(const octetStream& os)
{
    const octetStream* queued = 0;
    out.pop(queued);
    if (queued != &os){
        throw not_implemented();
    }
}