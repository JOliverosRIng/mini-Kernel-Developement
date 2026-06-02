
#include "memory.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>

MemoryManager::MemoryManager()
{
    blocks_.push_back({0, MEM_SIZE, -1, true});
}

// ─── allocate ─────────────────────────────────────────────────────────────────
// Algoritmo First-Fit: recorre blocks_ y asigna el primer bloque libre cuyo
// tamaño sea >= size. Si el bloque es más grande, lo divide en dos:
//   [bloque asignado (size)] [resto libre (bloque.size - size)]
int MemoryManager::allocate(int pid, int size)
{
    if (size <= 0 || size > MEM_SIZE)
    {
        std::cout << "[MemoryManager] ERROR: Tamaño inválido (" << size << ")\n";
        return -1;
    }

    for (auto it = blocks_.begin(); it != blocks_.end(); ++it)
    {
        if (!it->isFree || it->size < size)
        {
            continue;
        }

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

        std::cout << "[MemoryManager] Asignado " << size
                  << " unidades al PID " << pid
                  << " — dirección base: " << baseAddr << "\n";
        return baseAddr;
    }

    std::cout << "[MemoryManager] ERROR: Sin espacio contiguo para PID=" << pid
              << " (solicitado=" << size << " unidades)\n";
    return -1;
}

void MemoryManager::free(int pid)
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
        std::cout << "[MemoryManager] Liberada memoria del PID " << pid << "\n";
        coalesce();
    }
    else
    {
        std::cout << "[MemoryManager] WARN: No se encontró memoria para PID="
                  << pid << "\n";
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
    constexpr int COL = 44;
    const std::string LINE_D(COL, '=');
    const std::string LINE_S(COL, '-');

    std::cout << "\n+" << LINE_D << "+\n";
    std::cout << "|  MAPA DE MEMORIA (1024 unidades)           |\n";
    std::cout << "+" << LINE_D << "+\n";
    std::cout << std::left
              << "| " << std::setw(7) << "Inicio"
              << std::setw(7) << "Fin"
              << std::setw(8) << "Tamanio"
              << std::setw(6) << "PID"
              << std::setw(11) << "Estado"
              << "|\n";
    std::cout << "+" << LINE_S << "+\n";

    for (const auto &b : blocks_)
    {
        std::cout << "| "
                  << std::setw(7) << b.start
                  << std::setw(7) << (b.start + b.size - 1)
                  << std::setw(8) << b.size
                  << std::setw(6) << (b.isFree ? -1 : b.pid)
                  << std::setw(11) << (b.isFree ? "LIBRE" : "OCUPADO")
                  << "|\n";
    }

    int totalFree = 0, totalUsed = 0;
    for (const auto &b : blocks_)
    {
        if (b.isFree)
            totalFree += b.size;
        else
            totalUsed += b.size;
    }

    std::cout << "+" << LINE_S << "+\n";
    std::cout << "|  Libre: " << std::setw(4) << totalFree
              << "  Usado: " << std::setw(4) << totalUsed
              << "  Bloques: " << std::setw(3) << blocks_.size()
              << "           |\n";
    std::cout << "+" << LINE_D << "+\n\n";
}
