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
    IOType type;
    std::string fileName;
    int cyclesRemaining; // Ciclos restantes para completar
};

class IOManager
{
private:
    std::queue<IORequest> ioQueue_; // Cola de solicitudes pendientes
    IORequest *currentIO_;          // Solicitud en proceso (nullptr si ninguna)
    int completedOps_;              // Contador de operaciones completadas

    std::string typeToString(IOType type) const;

public:
    IOManager();
    ~IOManager();

    void requestIO(int pid, IOType type, const std::string &fileName);
    bool processIO(); // Simula un ciclo de E/S (retorna true si completó)
    bool isBusy() const;
    int queueSize() const;
    void printQueue() const;
    void printStatus() const;
};

#endif
