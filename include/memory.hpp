#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <vector>

static const int MEM_SIZE = 1024;

struct MemBlock
{
    int start;
    int size;
    int pid;
    bool isFree;
};

class MemoryManager
{
private:
    std::vector<MemBlock> blocks_;

    void coalesce();

public:
    MemoryManager();
    int allocate(int pid, int size);
    void free(int pid);
    void printMap() const;
};

#endif