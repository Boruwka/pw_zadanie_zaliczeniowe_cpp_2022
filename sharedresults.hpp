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
    uint64_t* results;
};

struct SharedForProcessesRemembered
{
    const uint64_t N = 10000;
    uint64_t remembered[10000];
    sem_t sem;
};



#endif
