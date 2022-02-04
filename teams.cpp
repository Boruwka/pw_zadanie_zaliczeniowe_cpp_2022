#include <utility>
#include <deque>
#include <future>
#include <queue>
#include <vector>

#include "teams.hpp"
#include "contest.hpp"
#include "collatz.hpp"

void calcSingleCollatz(InfInt singleInput, ContestResult& results, uint64_t idx, std::queue<uint64_t>& thread_queue, std::mutex& mut, bool share, std::shared_ptr<SharedResults> sharedResults)
{
    //printf("watek nr %d bede liczyc", idx);
    if (share)
    {
        results[idx] = calcCollatzWithShared(singleInput, sharedResults);
    }
    else
    {
        results[idx] = calcCollatz(singleInput);
    }
    mut.lock();
    thread_queue.push(idx);
    mut.unlock();
    //printf("watek nr %d policzylem", idx);
}

ContestResult TeamNewThreads::runContestImpl(ContestInput const & contestInput)
{
    ContestResult r;
    //printf("bede wykonywac mojo funkcje contest input o dlugosci %d\n", contestInput.size());
    r.resize(contestInput.size());
    // TeamNewThreads powinien tworzyć nowy wątek dla każdego wywołania calcCollatz, jednak nie więcej niż getSize() wątków jednocześnie.
    uint32_t max_num_of_threads = this->getSize();
    uint32_t current_num_of_threads = 0;
    uint64_t idx = 0;

    std::vector<std::thread> threads;
    std::mutex mut;
    std::queue<uint64_t> thread_queue;
    uint64_t input_size = contestInput.size();
    std::vector<bool> is_thread_joined(input_size, false);
    uint64_t num_of_joined_threads = 0;
    bool share = this->share;
    std::shared_ptr<SharedResults> sharedResults = this->getSharedResults();
        
    for(InfInt const & singleInput : contestInput)
    {
        if (current_num_of_threads == max_num_of_threads)
        {                
                mut.lock();
                uint64_t finished_idx = thread_queue.front();
                thread_queue.pop();
                mut.unlock();
                //printf("joinuje watek nr %d\n", finished_idx);
                threads[finished_idx].join();
                is_thread_joined[finished_idx] = true;
                num_of_joined_threads++;
            }
            std::thread t = createThread([singleInput, &r, idx, &thread_queue, &mut, share, sharedResults] { calcSingleCollatz(singleInput, r, idx, thread_queue, mut, share, sharedResults); });
            threads.push_back(std::move(t));
            idx++;
        }
        mut.lock();
        while (!thread_queue.empty())
        {
            idx = thread_queue.front();
            thread_queue.pop();
            mut.unlock();
            //printf("joinuje watek nr %d\n", idx);
            threads[idx].join();
            is_thread_joined[idx] = true;
            num_of_joined_threads++;
        mut.lock();  
    }
    mut.unlock();
    if (num_of_joined_threads != input_size)
    {
        for (uint64_t idx = 0; idx < input_size; idx++)
        {
            if (!is_thread_joined[idx])
            {
                //printf("joinuje watek nr %d\n", idx);
                threads[idx].join();
                is_thread_joined[idx] = true;
                num_of_joined_threads++;
            }
        }
    }
    //printf("wykonalom mojo funkcje\n");
    return r;
}

void calcAssignedCollatz(ContestInput const & contestInput, ContestResult& results, uint64_t idx, std::mutex& mut, uint64_t num_of_threads, bool share, std::shared_ptr<SharedResults> sharedResults)
{
    uint64_t input_size = contestInput.size();
    for (uint64_t i = idx; i < input_size; i += num_of_threads)
    {
        //printf("bede liczyc collatza dla indeksu %lu\n", i);
        uint64_t res;
        if (share)
        {
            res = calcCollatzWithShared(contestInput[i], sharedResults);
        }
        else
        {
            res = calcCollatz(contestInput[i]);
        }
        mut.lock();
        results[i] = res;
        mut.unlock();
    }
}

ContestResult TeamConstThreads::runContestImpl(ContestInput const & contestInput)
{
    ContestResult r;
    /* TeamConstThreads powinien utworzyć getSize() wątków, a każdy z wątków powinien wykonać podobną, zadaną z góry ilość pracy. */
    r.resize(contestInput.size());
    std::mutex mut;
    bool share = this->share;
    std::shared_ptr<SharedResults> sharedResults = this->getSharedResults();
    std::vector<std::thread> threads;
    uint64_t num_of_threads = this->getSize();
    //printf("num of threads = %lu\n", num_of_threads);
    for (int i = 0; i < num_of_threads; i++)
    {
        std::thread t = createThread([contestInput, &r, i, &mut, num_of_threads, share, sharedResults] { calcAssignedCollatz(contestInput, r, i, mut, num_of_threads, share, sharedResults); });
        threads.push_back(std::move(t));
    }
    for (int i = 0; i < num_of_threads; i++)
    {
        threads[i].join();
    }
    return r;
}

ContestResult TeamPool::runContest(ContestInput const & contestInput)
{
    ContestResult r;
    uint64_t num_of_threads = this->getSize();
    cxxpool::thread_pool pool{num_of_threads};
    uint64_t input_size = contestInput.size();
    std::vector<std::future<uint64_t>> futures;
    bool share = this->share;
    std::shared_ptr<SharedResults> sharedResults = this->getSharedResults();
    for (auto singleInput: contestInput)
    {
        if (share)
        {
            futures.push_back(pool.push([singleInput, sharedResults]{return calcCollatzWithShared(singleInput, sharedResults); }));
        }
        else
        {
            futures.push_back(pool.push([singleInput]{return calcCollatz(singleInput); }));
        }
    }
    for (uint64_t i = 0; i < input_size; i++)
    {
        r.push_back(futures[i].get());
    }
    return r;
}

ContestResult TeamNewProcesses::runContest(ContestInput const & contestInput)
{
    ContestResult r;
    //TODO
    return r;
}

ContestResult TeamConstProcesses::runContest(ContestInput const & contestInput)
{
    ContestResult r;
    //TODO
    return r;
}

ContestResult TeamAsync::runContest(ContestInput const & contestInput)
{
    ContestResult r;
    uint64_t input_size = contestInput.size();
    std::vector<std::future<uint64_t>> futures;
    bool share = this->share;
    std::shared_ptr<SharedResults> sharedResults = this->getSharedResults();
    for (auto singleInput: contestInput)
    {
        if (share)
        {
            futures.push_back(std::async([singleInput, sharedResults]{return calcCollatzWithShared(singleInput, sharedResults); }));
        }
        else
        {
            futures.push_back(std::async([singleInput]{return calcCollatz(singleInput); }));
        }
    }
    for (uint64_t i = 0; i < input_size; i++)
    {
        r.push_back(futures[i].get());
    }
    // auto futureLambda= std::async([](const std::string& s ){return "Hello C++11 from " + s + ".";},"lambda function")
    return r;
}
