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
    //std::cout << "bede wykonywac funkcje with shared dla n = " << n << "\n";
    uint64_t count = 0;
    assert(n > 0);

    if (n == 1)
    {
        return 0;
    }

    sharedResults->mut.lock();
    uint64_t res;
    if (sharedResults->map.count(n))
    {
        //std::cout << "znalezlismy wynik w mapie\n";
        res = sharedResults->map[n];
        sharedResults->mut.unlock();
        return res;
    }
    else
    {
        //std::cout << "nie ma wyniku w mapie\n";
        sharedResults->mut.unlock();
        InfInt new_n;
        if (n % 2 == 1)
        {
            new_n = InfInt(3) * n + 1;
        }
        else
        {
            new_n = n/2;
        }  
        res = calcCollatzWithShared(new_n, sharedResults) + 1;
        sharedResults->mut.lock();
        //std::cout << "zapisujemy wynik do mapy\n";
        sharedResults->map[n] = res;
        sharedResults->mut.unlock();
    }
    return res;

}

#endif
