#include "io.hpp"
#include <iostream>
#include <iomanip>

IOManager::IOManager() : currentIO_(nullptr), completedOps_(0)
{
    std::cout << "[IOManager] Gestor de E/S inicializado (latencia="
              << IO_TIME_CYCLES << " ciclos)\n";
}

IOManager::~IOManager()
{
    delete currentIO_;
}

std::string IOManager::typeToString(IOType type) const
{
    switch (type)
    {
    case IOType::READ:
        return "READ";
    case IOType::WRITE:
        return "WRITE";
    case IOType::OPEN:
        return "OPEN";
    case IOType::CLOSE:
        return "CLOSE";
    default:
        return "UNKNOWN";
    }
}

void IOManager::requestIO(int pid, IOType type, const std::string &fileName)
{
    IORequest req;
    req.pid = pid;
    req.type = type;
    req.fileName = fileName;
    req.cyclesRemaining = IO_TIME_CYCLES;

    ioQueue_.push(req);

    std::cout << "[IOManager] Solicitud recibida: PID=" << pid
              << " -> " << typeToString(type) << " '" << fileName << "'\n";
}

bool IOManager::processIO()
{
    // Si no hay operación en curso, tomar la siguiente de la cola
    if (currentIO_ == nullptr)
    {
        if (ioQueue_.empty())
        {
            return false; // No hay trabajo pendiente
        }

        currentIO_ = new IORequest(ioQueue_.front());
        ioQueue_.pop();

        std::cout << "[IOManager] Iniciando " << typeToString(currentIO_->type)
                  << " para PID=" << currentIO_->pid << " ('"
                  << currentIO_->fileName << "')\n";
    }

    // Procesar un ciclo de la operación actual
    currentIO_->cyclesRemaining--;

    std::cout << "[IOManager] Procesando... ciclos restantes: "
              << currentIO_->cyclesRemaining << "\n";

    // Si completó, liberar el dispositivo
    if (currentIO_->cyclesRemaining <= 0)
    {
        std::cout << "[IOManager] *** INTERRUPCION DE E/S *** Operación "
                  << typeToString(currentIO_->type) << " completada para PID="
                  << currentIO_->pid << "\n";

        completedOps_++;
        delete currentIO_;
        currentIO_ = nullptr;
        return true; // Operación completada
    }

    return false; // Aún procesando
}

bool IOManager::isBusy() const
{
    return currentIO_ != nullptr;
}

int IOManager::queueSize() const
{
    return static_cast<int>(ioQueue_.size());
}

void IOManager::printQueue() const
{
    const int ancho = 60;

    std::cout << "\n+" << std::string(ancho, '=') << "+\n";
    std::cout << "|  COLA DE E/S (" << ioQueue_.size()
              << " solicitudes pendientes)";

    int padding = ancho - 33;
    if (ioQueue_.size() >= 10)
        padding--;
    std::cout << std::string(padding, ' ') << "|\n";
    std::cout << "+" << std::string(ancho, '=') << "+\n";

    if (currentIO_ != nullptr)
    {
        std::cout << "| EN PROCESO:" << std::string(47, ' ') << "|\n";
        std::cout << "|   PID=" << currentIO_->pid << " -> "
                  << std::left << std::setw(6) << typeToString(currentIO_->type)
                  << " '" << currentIO_->fileName << "'";

        int fileLen = currentIO_->fileName.length();
        int pad = 35 - fileLen - (currentIO_->pid >= 10 ? 1 : 0);
        std::cout << std::string(pad, ' ') << "|\n";
        std::cout << "|   Ciclos restantes: " << currentIO_->cyclesRemaining
                  << std::string(36, ' ') << "|\n";
        std::cout << "+" << std::string(ancho, '-') << "+\n";
    }

    if (ioQueue_.empty())
    {
        std::cout << "| (Cola vacia)" << std::string(ancho - 15, ' ') << "|\n";
    }
    else
    {
        std::cout << "| PENDIENTES:" << std::string(47, ' ') << "|\n";

        std::queue<IORequest> copy = ioQueue_; // Copiar para iterar sin modificar
        int pos = 1;

        while (!copy.empty())
        {
            const IORequest &req = copy.front();
            std::cout << "|  " << pos << ". PID=" << req.pid << " -> "
                      << std::left << std::setw(6) << typeToString(req.type)
                      << " '" << req.fileName << "'";

            int fileLen = req.fileName.length();
            int pad = 33 - fileLen - (req.pid >= 10 ? 1 : 0) - (pos >= 10 ? 1 : 0);
            std::cout << std::string(pad, ' ') << "|\n";

            copy.pop();
            pos++;
        }
    }

    std::cout << "+" << std::string(ancho, '=') << "+\n\n";
}

void IOManager::printStatus() const
{
    std::cout << "[IOManager] Estado: ";
    if (currentIO_ != nullptr)
    {
        std::cout << "OCUPADO (PID=" << currentIO_->pid << ")";
    }
    else
    {
        std::cout << "DISPONIBLE";
    }
    std::cout << " | Cola: " << ioQueue_.size()
              << " | Completadas: " << completedOps_ << "\n";
}
