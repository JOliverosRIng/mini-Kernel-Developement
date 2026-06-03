#include "paging.hpp"

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

void PagingUnit::createTable(int pid, int memStart, int memSize, const std::string &processName)
{
    if (tables_.count(pid))
    {
        std::cout << "[PagingUnit] aviso: tabla para "
                  << describeProcess(pid, processName)
                  << " ya existe. Sobreescribiendo.\n";
    }

    std::unordered_map<int, int> table;

    int pageCount = (memSize + PAGE_SIZE - 1) / PAGE_SIZE;
    int baseFrame = memStart / PAGE_SIZE;

    for (int i = 0; i < pageCount; ++i)
    {
        table[i] = baseFrame + i;
    }

    tables_[pid] = std::move(table);

    std::cout << "[PagingUnit] tabla de paginas: " << processName 
              << " (" << pageCount << " paginas)\n";
}

void PagingUnit::removeTable(int pid, const std::string &processName)
{
    if (tables_.erase(pid))
    {
        std::cout << "[PagingUnit] tabla eliminada: "
                  << describeProcess(pid, processName) << "\n";
    }
    else
    {
        std::cout << "[PagingUnit] aviso: sin tabla para "
                  << describeProcess(pid, processName) << "\n";
    }
}

int PagingUnit::translate(int pid, int logicalAddr, const std::string &processName) const
{
    if (logicalAddr < 0)
    {
        std::cout << "[PagingUnit] error: dir logica negativa\n";
        return -1;
    }

    auto it = tables_.find(pid);
    if (it == tables_.end())
    {
        std::cout << "[PagingUnit] error: sin tabla para "
                  << describeProcess(pid, processName) << "\n";
        return -1;
    }

    int pageNum = logicalAddr / PAGE_SIZE;
    int offset = logicalAddr % PAGE_SIZE;

    const auto &table = it->second;
    auto fit = table.find(pageNum);
    if (fit == table.end() || fit->second == -1)
    {
        std::cout << "[PagingUnit] fallo de pagina: " << processName << "\n";
        return -1;
    }

    return fit->second * PAGE_SIZE + offset;
}

void PagingUnit::printTable(int pid, const std::string &processName) const
{
    auto it = tables_.find(pid);
    if (it == tables_.end())
    {
        std::cout << "[PagingUnit] No hay tabla de paginas para "
                  << describeProcess(pid, processName) << "\n";
        return;
    }

    std::cout << "\nTabla de paginas: " << processName << "\n";
    std::cout << std::left
              << std::setw(16) << "Pagina"
              << std::setw(16) << "Marco"
              << "Estado\n";

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

        std::cout << std::left
                  << std::setw(16) << fit->first
                  << std::setw(16) << fit->second
                  << "ok\n";
    }
}
