#include "paging.hpp"
#include <iostream>
#include <iomanip>

void PagingUnit::createTable(int pid, int memStart, int memSize)
{
    if (tables_.count(pid))
    {
        std::cout << "[PagingUnit] WARN: Tabla para PID=" << pid
                  << " ya existe. Sobreescribiendo.\n";
    }

    std::unordered_map<int, int> table;

    // Se calcula cuántas páginas necesita el proceso.
    int pageCount = (memSize + PAGE_SIZE - 1) / PAGE_SIZE;
    int baseFrame = memStart / PAGE_SIZE;

    // Mapeo lógico -> físico simplificado.
    for (int i = 0; i < pageCount; ++i)
    {
        table[i] = baseFrame + i;
    }

    tables_[pid] = std::move(table);

    std::cout << "[PagingUnit] Tabla creada para PID=" << pid
              << " — " << pageCount << " página(s)"
              << " — marco base=" << baseFrame << "\n";
}

void PagingUnit::removeTable(int pid)
{
    if (tables_.erase(pid))
    {
        std::cout << "[PagingUnit] Tabla eliminada para PID=" << pid << "\n";
    }
    else
    {
        std::cout << "[PagingUnit] WARN: No existe tabla para PID=" << pid << "\n";
    }
}

int PagingUnit::translate(int pid, int logicalAddr) const
{
    if (logicalAddr < 0)
    {
        std::cout << "[PagingUnit] ERROR: Dirección lógica negativa\n";
        return -1;
    }

    auto it = tables_.find(pid);
    if (it == tables_.end())
    {
        std::cout << "[PagingUnit] ERROR: Sin tabla para PID=" << pid << "\n";
        return -1;
    }

    int pageNum = logicalAddr / PAGE_SIZE;
    int offset = logicalAddr % PAGE_SIZE;

    const auto &table = it->second;
    auto fit = table.find(pageNum);
    if (fit == table.end() || fit->second == -1)
    {
        std::cout << "[PagingUnit] FALLO DE PÁGINA — PID=" << pid
                  << " dirección lógica=" << logicalAddr
                  << " página=" << pageNum << "\n";
        return -1;
    }

    int physicalAddr = fit->second * PAGE_SIZE + offset;

    std::cout << "[PagingUnit] Traducción PID=" << pid
              << " lógica=" << logicalAddr
              << " → página=" << pageNum
              << " marco=" << fit->second
              << " offset=" << offset
              << " física=" << physicalAddr << "\n";

    return physicalAddr;
}

void PagingUnit::printTable(int pid) const
{
    auto it = tables_.find(pid);
    if (it == tables_.end())
    {
        std::cout << "[PagingUnit] No hay tabla de páginas para PID=" << pid << "\n";
        return;
    }

    const std::string LINE(44, '=');
    const std::string LINE_S(44, '-');
    std::cout << "\n+" << LINE << "+\n";
    std::cout << "|  Tabla de Paginas - PID=" << std::setw(3) << pid
              << "                  |\n";
    std::cout << "+" << LINE << "+\n";
    std::cout << "| " << std::left
              << std::setw(14) << "Pag. Logica"
              << std::setw(14) << "Marco Fisico"
              << std::setw(16) << "Dir. Fisica"
              << "|\n";
    std::cout << "+" << LINE_S << "+\n";

    const auto &table = it->second;
    int maxPage = -1;
    for (const auto &[page, frame] : table)
    {
        if (page > maxPage)
            maxPage = page;
    }

    for (int p = 0; p <= maxPage; ++p)
    {
        auto fit = table.find(p);
        if (fit == table.end())
            continue;

        std::cout << "| "
                  << std::setw(14) << fit->first
                  << std::setw(14) << fit->second
                  << std::setw(16) << (fit->second * PAGE_SIZE)
                  << "|\n";
    }

    std::cout << "+" << std::string(44, '=') << "+\n\n";
}
