#ifndef PAGING_HPP
#define PAGING_HPP

#include <string>
#include <unordered_map>

static const int PAGE_SIZE = 64;

class PagingUnit
{
private:
    std::unordered_map<int, std::unordered_map<int, int>> tables_;

public:
    void createTable(int pid, int memStart, int memSize, const std::string &processName = "");
    void removeTable(int pid, const std::string &processName = "");
    int translate(int pid, int logicalAddr, const std::string &processName = "") const;
    void printTable(int pid, const std::string &processName = "") const;
};

#endif
