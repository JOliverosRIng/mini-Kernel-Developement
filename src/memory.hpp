#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <string>
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
    int allocate(int pid, int size, const std::string &processName = "");
    void free(int pid, const std::string &processName = "");
    void printMap() const;
};

#endif
