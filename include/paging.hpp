#ifndef PAGING_HPP
#define PAGING_HPP

#include <unordered_map>

static const int PAGE_SIZE = 64;

class PagingUnit
{
private:
    std::unordered_map<int, std::unordered_map<int, int>> tables_;

public:
    void createTable(int pid, int memStart, int memSize);
    void removeTable(int pid);
    int translate(int pid, int logicalAddr) const;
    void printTable(int pid) const;
};

#endif