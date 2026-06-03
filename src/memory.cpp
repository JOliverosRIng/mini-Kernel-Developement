#include "memory.hpp"

#include <iomanip>
#include <iostream>

namespace
{
std::string describeProcess(int pid, const std::string &name)
{
    if (!name.empty())
        return "Proceso '" + name + "'";
    return "Proceso " + std::to_string(pid);
}
} // namespace

MemoryManager::MemoryManager()
{
    blocks_.push_back({0, MEM_SIZE, -1, true});
}

int MemoryManager::allocate(int pid, int size, const std::string &processName)
{
    if (size <= 0 || size > MEM_SIZE)
    {
        std::cout << "[MemoryManager] error: tamano invalido (" << size << ")\n";
        return -1;
    }

    for (auto it = blocks_.begin(); it != blocks_.end(); ++it)
    {
        if (!it->isFree || it->size < size)
            continue;

        int baseAddr = it->start;

        if (it->size > size)
        {
            MemBlock remainder;
            remainder.start = it->start + size;
            remainder.size = it->size - size;
            remainder.pid = -1;
            remainder.isFree = true;

            it->size = size;
            it->pid = pid;
            it->isFree = false;

            blocks_.insert(it + 1, remainder);
        }
        else
        {
            it->pid = pid;
            it->isFree = false;
        }

        std::cout << "[MemoryManager] asignado: " << size
                  << " a " << processName << "\n";
        return baseAddr;
    }

    return -1;
}

void MemoryManager::free(int pid, const std::string &processName)
{
    bool found = false;

    for (auto &block : blocks_)
    {
        if (!block.isFree && block.pid == pid)
        {
            block.isFree = true;
            block.pid = -1;
            found = true;
        }
    }

    if (found)
    {
        std::cout << "[MemoryManager] memoria liberada para "
                  << processName << "\n";
        coalesce();
    }
    else
    {
        std::cout << "[MemoryManager] aviso: no hay memoria para "
                  << describeProcess(pid, processName) << "\n";
    }
}

void MemoryManager::coalesce()
{
    for (std::size_t i = 0; i + 1 < blocks_.size();)
    {
        if (blocks_[i].isFree && blocks_[i + 1].isFree)
        {
            blocks_[i].size += blocks_[i + 1].size;
            blocks_.erase(blocks_.begin() + static_cast<std::ptrdiff_t>(i) + 1);
        }
        else
        {
            ++i;
        }
    }
}

void MemoryManager::printMap() const
{
    std::cout << "\nMapa de memoria (1024 u):\n";
    std::cout << std::left
              << std::setw(7) << "Inicio"
              << std::setw(7) << "Fin"
              << std::setw(8) << "Tamano"
              << std::setw(10) << "Proceso"
              << "Estado\n";

    for (const auto &b : blocks_)
    {
        std::cout << std::left
                  << std::setw(7) << b.start
                  << std::setw(7) << (b.start + b.size - 1)
                  << std::setw(8) << b.size
                  << std::setw(10) << (b.isFree ? -1 : b.pid)
                  << (b.isFree ? "libre" : "ocupado") << "\n";
    }

    int totalFree = 0, totalUsed = 0;
    for (const auto &b : blocks_)
    {
        if (b.isFree)
            totalFree += b.size;
        else
            totalUsed += b.size;
    }

    std::cout << " libre: " << totalFree
              << " - usado: " << totalUsed
              << " - bloques: " << blocks_.size() << "\n";
}
