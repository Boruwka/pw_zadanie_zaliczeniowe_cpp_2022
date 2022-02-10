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
    const uint64_t N = 1000000;
    uint64_t remembered[1000000];
    sem_t sem;
};



#endif
