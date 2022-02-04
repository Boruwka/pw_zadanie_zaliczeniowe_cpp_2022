#ifndef COLLATZ_HPP
#define COLLATZ_HPP

#include <assert.h>
#include <memory>

#include "sharedresults.hpp"

inline uint64_t calcCollatz(InfInt n)
{
    // It's ok even if the value overflow
    uint64_t count = 0;
    assert(n > 0);

    while (n != 1)
    {
        ++count;
        if (n % 2 == 1)
        {
            n *= 3;
            n += 1;
        }
        else
        {
            n /= 2;
        }            
    }

    return count;
}

inline uint64_t calcCollatzWithShared(InfInt n, std::shared_ptr<SharedResults> sharedResults)
{
    // It's ok even if the value overflow
    uint64_t count = 0;
    assert(n > 0);

    if (n == 1)
    {
        return 0;
    }

    InfInt new_n;
    if (n % 2 == 1)
    {
        new_n = InfInt(3) * n + 1;
    }
    else
    {
        new_n = n/2;
    }  
    
    uint64_t res;          
    
    if (sharedResults->map.count(new_n))
    {
        res = sharedResults->map[new_n] + 1;
    }
    else
    {
        res = calcCollatz(new_n) + 1;
    }

    sharedResults->map[n] = res;
    return res;

}

#endif
