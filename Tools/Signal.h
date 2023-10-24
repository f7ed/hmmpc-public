#ifndef TOOLS_SIGNAL_H_
#define TOOLS_SIGNAL_H_

#include <pthread.h>

class Signal
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
public:
    Signal(); //inital a Signal
    virtual ~Signal(); // destroy it
    void lock(); //lock the mutex
    void unlock(); //unlock the mutex
    void wait(); //wait if the cond is false
    int wait(int seconds); //wait if the cond is false or timeout
    void broadcast(); //broadcast the threads which is waiting the cond
};
#endif