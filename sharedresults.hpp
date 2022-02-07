#ifndef SHAREDRESULTS_HPP
#define SHAREDRESULTS_HPP

#include <map>
#include <mutex> 
#include <semaphore.h>

class SharedResults
{
    public:
    SharedResults() {}
    std::map<InfInt, uint64_t> map;
    std::mutex mut;
};

struct SharedForProcesses
{
    //unsigned long int* results; // jak zadeklarować tu tablicę? 
    sem_t sem; // semafor chroniący dostęp do results
    uint64_t* results;
};


#endif
