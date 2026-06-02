#ifndef PAGING_HPP
#define PAGING_HPP

#include <unordered_map>

constexpr int PAGE_SIZE = 64;
constexpr int NUM_FRAMES = 16;

class PagingUnit
{
public:
    void createTable(int pid, int memStart, int memSize);
    void removeTable(int pid);

    int translate(int pid, int logicalAddr) const;

    void printTable(int pid) const;

private:
    std::unordered_map<int, std::unordered_map<int, int>> tables_;
};

#endif
