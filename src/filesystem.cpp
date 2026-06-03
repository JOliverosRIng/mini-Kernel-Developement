#include "filesystem.hpp"
#include <iostream>
#include <iomanip>

FileSystem::FileSystem() : fileCount_(0)
{
    std::cout << "[FileSystem] Sistema de archivos inicializado (max "
              << MAX_FILES << " archivos)\n";
}

bool FileSystem::create(const std::string &name, int size, int pid)
{
    // Validaciones básicas
    if (name.empty())
    {
        std::cout << "[FileSystem] ERROR: Nombre de archivo vacío\n";
        return false;
    }

    if (size <= 0 || size > MAX_FILE_SIZE)
    {
        std::cout << "[FileSystem] ERROR: Tamaño inválido (" << size
                  << "). Debe estar entre 1 y " << MAX_FILE_SIZE << "\n";
        return false;
    }

    if (fileCount_ >= MAX_FILES)
    {
        std::cout << "[FileSystem] ERROR: Sistema de archivos lleno (max "
                  << MAX_FILES << " archivos)\n";
        return false;
    }

    // Verificar si ya existe
    if (directory_.find(name) != directory_.end())
    {
        std::cout << "[FileSystem] ERROR: El archivo '" << name
                  << "' ya existe\n";
        return false;
    }

    // Crear el FCB
    FCB fcb;
    fcb.name = name;
    fcb.size = size;
    fcb.createdBy = pid;
    fcb.isOpen = false;
    fcb.openedBy = -1;

    directory_[name] = fcb;
    fileCount_++;

    std::cout << "[FileSystem] Archivo creado: '" << name << "' ("
              << size << " bytes) por PID=" << pid << "\n";
    return true;
}

bool FileSystem::open(const std::string &name, int pid)
{
    auto it = directory_.find(name);

    if (it == directory_.end())
    {
        std::cout << "[FileSystem] ERROR: Archivo '" << name
                  << "' no encontrado\n";
        return false;
    }

    if (it->second.isOpen)
    {
        std::cout << "[FileSystem] ERROR: Archivo '" << name
                  << "' ya está abierto por PID=" << it->second.openedBy << "\n";
        return false;
    }

    it->second.isOpen = true;
    it->second.openedBy = pid;

    std::cout << "[FileSystem] Archivo abierto: '" << name
              << "' por PID=" << pid << "\n";
    return true;
}

bool FileSystem::close(const std::string &name, int pid)
{
    auto it = directory_.find(name);

    if (it == directory_.end())
    {
        std::cout << "[FileSystem] ERROR: Archivo '" << name
                  << "' no encontrado\n";
        return false;
    }

    if (!it->second.isOpen)
    {
        std::cout << "[FileSystem] ERROR: Archivo '" << name
                  << "' no está abierto\n";
        return false;
    }

    if (it->second.openedBy != pid)
    {
        std::cout << "[FileSystem] ERROR: PID=" << pid
                  << " no tiene permiso para cerrar '" << name
                  << "' (abierto por PID=" << it->second.openedBy << ")\n";
        return false;
    }

    it->second.isOpen = false;
    it->second.openedBy = -1;

    std::cout << "[FileSystem] Archivo cerrado: '" << name
              << "' por PID=" << pid << "\n";
    return true;
}

bool FileSystem::remove(const std::string &name, int pid)
{
    auto it = directory_.find(name);

    if (it == directory_.end())
    {
        std::cout << "[FileSystem] ERROR: Archivo '" << name
                  << "' no encontrado\n";
        return false;
    }

    if (it->second.isOpen)
    {
        std::cout << "[FileSystem] ERROR: No se puede eliminar '" << name
                  << "' (está abierto por PID=" << it->second.openedBy << ")\n";
        return false;
    }

    directory_.erase(it);
    fileCount_--;

    std::cout << "[FileSystem] Archivo eliminado: '" << name
              << "' por PID=" << pid << "\n";
    return true;
}

bool FileSystem::exists(const std::string &name) const
{
    return directory_.find(name) != directory_.end();
}

void FileSystem::printDirectory() const
{
    const int ancho = 70;

    std::cout << "\n+" << std::string(ancho, '=') << "+\n";
    std::cout << "|  DIRECTORIO RAIZ (" << fileCount_ << "/" << MAX_FILES
              << " archivos)";
    std::cout << std::string(ancho - 28 - (fileCount_ >= 10 ? 1 : 0), ' ') << "|\n";
    std::cout << "+" << std::string(ancho, '=') << "+\n";

    if (directory_.empty())
    {
        std::cout << "| " << std::string(ancho - 2, ' ') << "|\n";
        std::cout << "|  (Directorio vacio)"
                  << std::string(ancho - 22, ' ') << "|\n";
        std::cout << "| " << std::string(ancho - 2, ' ') << "|\n";
    }
    else
    {
        std::cout << "| Nombre" << std::string(14, ' ')
                  << "Tamanio  Creado  Estado"
                  << std::string(13, ' ') << "|\n";
        std::cout << "+" << std::string(ancho, '-') << "+\n";

        for (const auto &entry : directory_)
        {
            const FCB &fcb = entry.second;

            std::cout << "| " << std::left << std::setw(20) << fcb.name;
            std::cout << std::right << std::setw(6) << fcb.size << "B  ";
            std::cout << "PID=" << std::setw(2) << fcb.createdBy << "  ";

            if (fcb.isOpen)
            {
                std::cout << "ABIERTO (PID=" << fcb.openedBy << ")";
                int padding = 16 - (fcb.openedBy >= 10 ? 1 : 0);
                std::cout << std::string(padding, ' ') << "|\n";
            }
            else
            {
                std::cout << "CERRADO" << std::string(16, ' ') << "|\n";
            }
        }
    }

    std::cout << "+" << std::string(ancho, '=') << "+\n\n";
}
