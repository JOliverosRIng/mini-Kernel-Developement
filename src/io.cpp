#include "io.hpp"

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

IOManager::IOManager() : isProcessing_(false), completedOps_(0)
{
    std::cout << "[IOManager] Gestor de E/S inicializado (latencia="
              << IO_TIME_CYCLES << " ciclos)\n";
}

IOManager::~IOManager() {}

std::string IOManager::typeToString(IOType type) const
{
    switch (type)
    {
    case IOType::READ:
        return "lectura";
    case IOType::WRITE:
        return "escritura";
    case IOType::OPEN:
        return "apertura";
    case IOType::CLOSE:
        return "cierre";
    default:
        return "desconocido";
    }
}

void IOManager::requestIO(int pid, IOType type, const std::string &fileName, const std::string &processName)
{
    IORequest req;
    req.pid = pid;
    req.processName = processName;
    req.type = type;
    req.fileName = fileName;
    req.cyclesRemaining = IO_TIME_CYCLES;

    ioQueue_.push(req);

    std::cout << "[IOManager] Solicitud recibida: "
              << describeProcess(pid, processName)
              << " -> " << typeToString(type) << " '" << fileName << "'\n";
}

bool IOManager::processIO()
{
    return processAndGetCompletedPid() > 0;
}

int IOManager::processAndGetCompletedPid()
{
    if (!isProcessing_)
    {
        if (ioQueue_.empty())
            return -1;

        activeIO_ = ioQueue_.front();
        ioQueue_.pop();
        isProcessing_ = true;

        std::cout << "[IOManager] Iniciando " << typeToString(activeIO_.type)
                  << " para " << describeProcess(activeIO_.pid, activeIO_.processName) << " ('"
                  << activeIO_.fileName << "')\n";
    }

    activeIO_.cyclesRemaining--;

    std::cout << "[IOManager] Procesando... ciclos restantes: "
              << activeIO_.cyclesRemaining << "\n";

    if (activeIO_.cyclesRemaining <= 0)
    {
        std::cout << "[IOManager] interrupcion: "
                  << typeToString(activeIO_.type) << " completada para "
                  << activeIO_.processName << "\n";

        completedOps_++;
        int completedPid = activeIO_.pid;
        isProcessing_ = false;
        return completedPid;
    }

    return 0;
}

bool IOManager::isBusy() const
{
    return isProcessing_;
}

int IOManager::queueSize() const
{
    return static_cast<int>(ioQueue_.size());
}

void IOManager::printQueue() const
{
    std::cout << "\nEstado de e/s (" << ioQueue_.size() << " pendientes):\n";

    if (isProcessing_)
    {
        std::cout << " en proceso:\n";
        std::cout << "   " << describeProcess(activeIO_.pid, activeIO_.processName) << " -> "
                  << std::left << std::setw(6) << typeToString(activeIO_.type)
                  << " '" << activeIO_.fileName << "'\n";
        std::cout << "   ciclos restantes: " << activeIO_.cyclesRemaining << "\n";
    }

    if (ioQueue_.empty())
    {
        std::cout << " (cola vacia)\n";
    }
    else
    {
        std::cout << " pendientes:\n";
        std::queue<IORequest> copy = ioQueue_;
        int pos = 1;
        while (!copy.empty())
        {
            const IORequest &req = copy.front();
            std::cout << "  " << pos << ". " << describeProcess(req.pid, req.processName) << " -> "
                      << std::left << std::setw(6) << typeToString(req.type)
                      << " '" << req.fileName << "'\n";

            copy.pop();
            pos++;
        }
    }
}

void IOManager::printStatus() const
{
    std::cout << "[IOManager] estado: ";
    if (isProcessing_)
        std::cout << "ocupado (" << activeIO_.processName << ")";
    else
        std::cout << "disponible";
    std::cout << " - cola: " << ioQueue_.size() << " - completadas: " << completedOps_ << "\n";
}
