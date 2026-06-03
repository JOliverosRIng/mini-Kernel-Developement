#ifndef IO_HPP
#define IO_HPP

#include <string>
#include <queue>

static const int IO_TIME_CYCLES = 3; // Ciclos que tarda una operación de E/S

enum class IOType
{
    READ,
    WRITE,
    OPEN,
    CLOSE
};

// Solicitud de E/S
struct IORequest
{
    int pid;
    std::string processName;
    IOType type;
    std::string fileName;
    int cyclesRemaining; // Ciclos restantes para completar
};

class IOManager
{
private:
    std::queue<IORequest> ioQueue_; // Cola de solicitudes pendientes
    IORequest activeIO_;            // Datos de la solicitud en curso
    bool isProcessing_;             // Indica si hay una operacion activa
    int completedOps_;              // Contador de operaciones completadas

    std::string typeToString(IOType type) const;

public:
    IOManager();
    ~IOManager();

    void requestIO(int pid, IOType type, const std::string &fileName, const std::string &processName = "");
    bool processIO(); // Simula un ciclo de E/S (retorna true si completó)
    // Procesa un ciclo y retorna el PID cuya operación se completó (>0),
    // retorna 0 si no se completó ninguna en este ciclo, y -1 si no hay trabajo.
    int processAndGetCompletedPid();
    bool isBusy() const;
    int queueSize() const;
    void printQueue() const;
    void printStatus() const;
};

#endif
