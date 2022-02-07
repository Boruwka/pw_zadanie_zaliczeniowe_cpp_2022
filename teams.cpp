#include <utility>
#include <deque>
#include <future>
#include <queue>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/stat.h> 
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

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
    // printf("team new threads share = %d\n", share);
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
    uint64_t max_num_of_procs = this->getSize();
    uint64_t current_num_of_procs = 0;
    uint64_t input_size = contestInput.size();
    //SharedForProcesses* shared = malloc(sizeof(SharedForProcesses)); // chyba nie tak
    int fd_memory = -1;
    int flags, prot;
    prot = PROT_READ | PROT_WRITE;
    flags = MAP_SHARED | MAP_ANONYMOUS;
    void* mapped_mem = mmap(NULL, input_size * sizeof(uint64_t) + sizeof(SharedForProcesses), prot, flags, fd_memory, 0);
    SharedForProcesses* shared = (SharedForProcesses*)(mapped_mem + input_size * sizeof(uint64_t));
    shared->results = (uint64_t*)(mapped_mem);
    //SharedForProcesses* shared = (SharedForProcesses*)mmap(NULL, sizeof(SharedForProcesses), prot, flags, fd_memory, 0);
    //fprintf(stderr, "utworzone mmap\n");
    // shared->results = (unsigned long int*)malloc(input_size*sizeof(unsigned long long)); // to się chyba jeszcze przyda 
    //fprintf(stderr, "zaalokowane\n");
    sem_init(&(shared->sem), 1, 1);
    //fprintf(stderr, "semafor zainijcalizowany\n");
    // przyłączanie mapped_mem
    for (int i = 0; i < input_size; i++)
    {
        //fprintf(stderr, "pentla, i = %d\n", i);
        if (current_num_of_procs == max_num_of_procs)
        {
            wait(NULL); // nie wiem, czy to czeka tylko na jednego
            current_num_of_procs--;
        }
        current_num_of_procs++;
        pid_t pid = fork();
        if (pid == 0)
        {
            // jestem potomkiem
            //sem_wait(&shared->sem);
            shared->results[i] = calcCollatz(contestInput[i]);
            //printf("bedziemy wrzucac wynik %lu na pozycje %d\n", shared->results[i], i);
            //sem_post(&shared->sem);
            // chyba nawet nie potrzebuję tu semafora
            exit(0);
        }
    }
    int wpid;
    while ((wpid = wait(NULL)) > 0); // czekamy na wszystkie dzieci
    for (int i = 0; i < input_size; i++)
    {
        //printf("wrzucamy do wektora %lu\n", shared->results[i]);
        r.push_back(shared->results[i]);
    }
    return r;
}

ContestResult TeamConstProcesses::runContest(ContestInput const & contestInput)
{
    ContestResult r;
    uint64_t num_of_procs = this->getSize();
    uint64_t input_size = contestInput.size();
    //SharedForProcesses* shared = malloc(sizeof(SharedForProcesses)); // chyba nie tak
    int fd_memory = -1;
    int flags, prot;
    prot = PROT_READ | PROT_WRITE;
    flags = MAP_SHARED | MAP_ANONYMOUS;
    void* mapped_mem = mmap(NULL, input_size * sizeof(uint64_t) + sizeof(SharedForProcesses), prot, flags, fd_memory, 0);
    SharedForProcesses* shared = (SharedForProcesses*)(mapped_mem + input_size * sizeof(uint64_t));
    shared->results = (uint64_t*)(mapped_mem);
    sem_init(&(shared->sem), 1, 1);
    // przyłączanie mapped_mem
    for (int i = 0; i < num_of_procs; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            // jestem potomkiem
            for (int j = i; j < input_size; j+=num_of_procs)
            {
                shared->results[j] = calcCollatz(contestInput[j]);
            }
            exit(0);
        }
    }
    int wpid;
    while ((wpid = wait(NULL)) > 0); // czekamy na wszystkie dzieci
    for (int i = 0; i < input_size; i++)
    {
        //printf("wrzucamy do wektora %lu\n", shared->results[i]);
        r.push_back(shared->results[i]);
    }
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
    return r;
}
