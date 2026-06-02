#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <vector>
#include <string>

constexpr int MEM_SIZE = 1024;

struct MemBlock
{
    int start;
    int size;
    int pid;
    bool isFree;
};

class MemoryManager
{
public:
    MemoryManager();

    int allocate(int pid, int size);
    void free(int pid);
    void printMap() const;

private:
    std::vector<MemBlock> blocks_; // Partición actual de la memoria
    void coalesce();
};

#endif
