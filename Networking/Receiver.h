#ifndef NETWORKING_RECEIVER_H_
#define NETWORKING_RECEIVER_H_

#include <pthread.h>

#include "Tools/octetStream.h"
#include "Tools/WaitQueue.h"
#include "Tools/time-func.h"

class Receiver
{
    int socket;
    WaitQueue<octetStream*> in;/* to recieve */
    WaitQueue<octetStream*> out;/* recieved */
    pthread_t thread;

    static void* run_thread(void* receiver);

    Receiver(const Receiver& other);

    void start();/* Create a thread to execute run()(receive data). */
    void stop();
    void run();/* Receive data in the thread. */

public:
    Timer timer;
    Receiver(int socket);/* Construct a new Receiver to receive data via the socket. */
    ~Receiver();

    void request(octetStream& os);/* Request a octetStream to receive  */
    void wait(octetStream& os);
};

#endif
