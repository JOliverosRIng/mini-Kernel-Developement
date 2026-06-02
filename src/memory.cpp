#include "memory.hpp"
#include <iostream>
#include <iomanip>

MemoryManager::MemoryManager()
{
    // Estado inicial: toda la memoria está libre en un único bloque.
    blocks_.push_back({0, MEM_SIZE, -1, true});
}

int MemoryManager::allocate(int pid, int size)
{
    // Validación básica del tamaño solicitado.
    if (size <= 0 || size > MEM_SIZE)
    {
        std::cout << "[MemoryManager] ERROR: Tamaño inválido (" << size << ")\n";
        return -1;
    }

    // First-Fit: busca el primer bloque libre suficientemente grande.
    for (auto it = blocks_.begin(); it != blocks_.end(); ++it)
    {
        if (!it->isFree || it->size < size)
        {
            continue;
        }

        int baseAddr = it->start;

        // Si el bloque es más grande que lo pedido, se divide en dos:
        // una parte ocupada y una parte libre restante.
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
            // Si el bloque encaja exactamente, solo se marca como ocupado.
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

    // Busca el bloque perteneciente al proceso y lo marca como libre.
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
    // Fusiona bloques libres contiguos para reducir fragmentación externa.
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