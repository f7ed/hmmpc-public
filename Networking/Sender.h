#ifndef NETWORKING_SENDER_H_
#define NETWORKING_SENDER_H_

#include <pthread.h>

#include "Tools/octetStream.h"
#include "Tools/time-func.h"
#include "Tools/WaitQueue.h"

class Sender
{
    int socket;
    WaitQueue<const octetStream*> in;/* to send */
    WaitQueue<const octetStream*> out;/* sent */
    pthread_t thread;

    static void* run_thread(void* sender);

    // prevent copying
    Sender(const Sender& others);

    void start();/* Create a thread to execute run()(send data). */
    void stop();
    void run();/* Sends all octetStream in "in queue"(to send) in the thread. */

public:
    Timer timer;
    Sender(int socket);/* Construct a new Sender to send data via the socket. */
    ~Sender();

    void request(const octetStream& os);/* Request a octetStream to send. */
    void wait(const octetStream& os);
};
#endif