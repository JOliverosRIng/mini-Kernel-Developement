#include "filesystem.hpp"

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

FileSystem::FileSystem() : fileCount_(0)
{
    std::cout << "[FileSystem] fs inicializado (max "
              << MAX_FILES << " archivos)\n";
}

bool FileSystem::create(const std::string &name, int size, int pid, const std::string &processName)
{
    if (name.empty())
    {
        std::cout << "[FileSystem] error: nombre vacio\n";
        return false;
    }

    if (size <= 0 || size > MAX_FILE_SIZE)
    {
        std::cout << "[FileSystem] error: tamano invalido (" << size << ")\n";
        return false;
    }

    if (fileCount_ >= MAX_FILES)
    {
        std::cout << "[FileSystem] error: fs lleno\n";
        return false;
    }

    if (directory_.find(name) != directory_.end())
    {
        std::cout << "[FileSystem] error: '" << name << "' ya existe\n";
        return false;
    }

    FCB fcb;
    fcb.name = name;
    fcb.size = size;
    fcb.createdBy = pid;
    fcb.createdByName = processName;
    fcb.isOpen = false;
    fcb.openedBy = -1;
    fcb.openedByName.clear();

    directory_[name] = fcb;
    fileCount_++;

    std::cout << "[FileSystem] archivo creado: '" << name << "' por " << processName << "\n";
    return true;
}

bool FileSystem::open(const std::string &name, int pid, const std::string &processName)
{
    auto it = directory_.find(name);

    if (it == directory_.end())
    {
        std::cout << "[FileSystem] error: '" << name << "' no encontrado\n";
        return false;
    }

    if (it->second.isOpen)
    {
        std::cout << "[FileSystem] error: '" << name << "' ya abierto por "
                  << it->second.openedByName << "\n";
        return false;
    }

    it->second.isOpen = true;
    it->second.openedBy = pid;
    it->second.openedByName = processName;
    
    std::cout << "[FileSystem] archivo abierto: '" << name << "'\n";
    return true;
}

bool FileSystem::close(const std::string &name, int pid, const std::string &processName)
{
    auto it = directory_.find(name);

    if (it == directory_.end())
    {
        std::cout << "[FileSystem] error: '" << name << "' no encontrado\n";
        return false;
    }

    if (!it->second.isOpen)
    {
        std::cout << "[FileSystem] error: '" << name << "' no abierto\n";
        return false;
    }

    if (it->second.openedBy != pid)
    {
        std::cout << "[FileSystem] error: "
                  << describeProcess(pid, processName)
                  << " no es dueno de '" << name << "'\n";
        return false;
    }

    it->second.isOpen = false;
    it->second.openedBy = -1;
    it->second.openedByName.clear();
    
    std::cout << "[FileSystem] archivo cerrado: '" << name << "'\n";
    return true;
}

bool FileSystem::remove(const std::string &name, int pid, const std::string &processName)
{
    (void)pid; // Evitar advertencia de parametro no usado

    auto it = directory_.find(name);

    if (it == directory_.end())
    {
        std::cout << "[FileSystem] error: '" << name << "' no encontrado\n";
        return false;
    }

    if (it->second.isOpen)
    {
        std::cout << "[FileSystem] error: no se puede eliminar '" << name << "' abierto\n";
        return false;
    }

    directory_.erase(it);
    fileCount_--;
    
    std::cout << "[FileSystem] archivo eliminado: '" << name << "' por " << processName << "\n";
    return true;
}

bool FileSystem::exists(const std::string &name) const
{
    return directory_.find(name) != directory_.end();
}

void FileSystem::printDirectory() const
{
    std::cout << "\nDirectorio (" << fileCount_ << "/" << MAX_FILES << "):\n";

    if (directory_.empty())
    {
        std::cout << " (vacio)\n";
    }
    else
    {
        std::cout << std::left << std::setw(20) << "Nombre"
                  << "Tamano  Creado  Estado"
                  << "\n";

        for (const auto &entry : directory_)
        {
            const FCB &fcb = entry.second;

            std::cout << std::left << std::setw(20) << fcb.name;
            std::cout << std::right << std::setw(6) << fcb.size << "B  ";
            std::cout << std::left << std::setw(18) << describeProcess(fcb.createdBy, fcb.createdByName) << "  ";

            if (fcb.isOpen)
            {
                std::string owner = describeProcess(fcb.openedBy, fcb.openedByName);
                std::cout << "abierto (" << owner << ")\n";
            }
            else
            {
                std::cout << "cerrado\n";
            }
        }
    }
}
