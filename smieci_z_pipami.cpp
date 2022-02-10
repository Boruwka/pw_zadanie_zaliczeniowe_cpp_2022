ContestResult TeamConstProcesses::runContest(ContestInput const & contestInput)
{
    ContestResult r;
    uint64_t num_of_procs = this->getSize();
    uint64_t input_size = contestInput.size();
    int fd_memory = -1;
    int flags, prot;
    prot = PROT_READ | PROT_WRITE;
    flags = MAP_SHARED | MAP_ANONYMOUS;
    void* mapped_mem;
    SharedForProcesses* shared;
    SharedForProcessCommunication* communication;
    if (this->share)
    {
        mapped_mem = mmap(NULL, input_size * sizeof(uint64_t) + sizeof(SharedForProcesses) +  num_of_procs * sizeof(uint64_t) + 2 * num_of_procs * sizeof(std::pair<uint64_t, bool>*) + sizeof(SharedForProcessCommunication), prot, flags, fd_memory, 0);
        communication = (SharedForProcessCommunication*)(mapped_mem + input_size * sizeof(uint64_t) + sizeof(SharedForProcesses) +  num_of_procs * sizeof(uint64_t) + 2 * num_of_procs * sizeof(std::pair<uint64_t, bool>*));
        communication->answers = (std::pair<uint64_t, bool>*)(mapped_mem + input_size * sizeof(uint64_t) + sizeof(SharedForProcesses) +  num_of_procs * sizeof(uint64_t) + num_of_procs * sizeof(std::pair<uint64_t, bool>*));
        communication->questions = (std::pair<uint64_t, bool>*)(mapped_mem + input_size * sizeof(uint64_t) + sizeof(SharedForProcesses) +  num_of_procs * sizeof(uint64_t));
        communication->thread_queue = (uint64_t*)(mapped_mem + input_size * sizeof(uint64_t) + sizeof(SharedForProcesses));
        shared = (SharedForProcesses*)(mapped_mem + input_size * sizeof(uint64_t));
        shared->results = (uint64_t*)(mapped_mem);
        communication->queue_positon = 0;
        sem_init(&(communication->sem), 1, 1);
    }
    else
    {
        mapped_mem = mmap(NULL, input_size * sizeof(uint64_t) + sizeof(SharedForProcesses), prot, flags, fd_memory, 0);
        SharedForProcesses* shared = (SharedForProcesses*)(mapped_mem + input_size * sizeof(uint64_t));
        shared->results = (uint64_t*)(mapped_mem);
    }
    //sem_init(&(shared->sem), 1, 1);
    // przyłączanie mapped_mem
    for (int i = 0; i < num_of_procs; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            // jestem potomkiem
            if (this->share)
            {
                for (int j = i; j < input_size; j+=num_of_procs)
                {
                    shared->results[j] = calcCollatzWithProcessComunication(contestInput[j], communication);
                }
            }
            else
            {
                for (int j = i; j < input_size; j+=num_of_procs)
                {
                    shared->results[j] = calcCollatz(contestInput[j]);
                }
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


struct SharedForProcessCommunication
{
    uint64_t* proc_queue;
    uint64_t queue_position;
    std::pair<uint64_t, bool>* questions; // w jakim indeksie, i czy mam zapisać liczbę (true), czy ją odczytać (false)
    std::pair<uint64_t, bool>* answers; // tyko jeśli zapytanie dotyczy odczytywania liczby, w pierwszym liczba, w drugim true jeśli była, false jeśli nie było (i wtedy wartość w pierwszym nie ma mieć sensu)
    sem_t sem;
};


inline uint64_t calcCollatzWithProcessCommunication(InfInt n, CommunicationForProcesses* communication, uint64_t idx)
{
    //std::cout << "bede wykonywac funkcje with shared dla n = " << n << "\n";
    uint64_t count = 0;
    assert(n > 0);

    if (n == 1)
    {
        return 0;
    }

    uint64_t res;
    
    sem_wait(communication->sem);
    communication->questions[idx] = std::make_pair(n, false);
    communication->proc_queue[communication->queue_position] = idx;
    communication->queue_postion++;
    sem_post(communication->sem);
    sem_wait(communication->for_answer[idx]);
    if (communication->answers[idx].second)
    {
        //std::cout << "znalezlismy wynik w mapie\n";
        res = communication->answers[idx].first;
        return res;
    }
    else
    {
        //std::cout << "nie ma wyniku w mapie\n";
        InfInt new_n;
        if (n % 2 == 1)
        {
            new_n = InfInt(3) * n + 1;
        }
        else
        {
            new_n = n/2;
        }  
        res = calcCollatzWithProcessCommunication(new_n, communication) + 1;
        //std::cout << "zapisujemy wynik do mapy\n";
        sem_wait(communication->sem);
        communication->questions[idx] = std::make_pair(res, true);
        communication->proc_queue[communication->queue_position] = idx;
        communication->queue_postion++;
        sem_post(communication->sem);
    }
    return res;

}
