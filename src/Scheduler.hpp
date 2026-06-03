//SCHEDULER

// Administra la cola de procesos READY y les asigna CPU
// por turnos de duración fija (quantum). Compatible con
// el estado BLOCKED de ProcessState.
// Implementa el algoritmo Round-Robin: cada proceso recibe un
// quantum completo, luego se reencola al final de la cola de listos
// Flujo:
//   enqueue(): proceso pasa a READY, entra a la cola
//   tick(): avanza UN tick de CPU (llama decrementTime)
//   runQuantum(): ejecuta un quantum completo sobre un proceso
//   runAll():ciclo completo hasta que todos terminen

#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "process.hpp"
#include <queue>
#include <vector>
#include <string>
#include <functional>


class Scheduler
{
private:
    std::queue<Process *> readyQueue_; // cola FIFO de punteros a procesos
    std::vector<Process *> blocked_;   // procesos en estado BLOCKED
    int quantum_;                      // ticks máximos por turno
    int clock_;                        // reloj global de la simulación

    // Imprime una línea de separación con título
    void printSection(const std::string &title) const;

public:
    // quantum por defecto: 3 ticks
    explicit Scheduler(int quantum = 3);

    // ── Interfaz pública ──────────────────────────────────────

    // Agrega un proceso a la cola → lo pone en READY
    void enqueue(Process *p);

    // Desbloquea un proceso BLOCKED y lo reencola → READY
    void unblock(Process *p);

    // Ejecuta UN quantum sobre el proceso al frente de la cola
    // Retorna true si todavía hay procesos en la cola
    bool runQuantum();

    // Resultado detallado de la ejecución de un quantum
    struct QuantumResult
    {
        Process *proc;     // proceso ejecutado (puede ser nullptr)
        bool terminated;   // true si el proceso terminó durante el quantum
        int ticksExecuted; // número de ticks ejecutados en el quantum
    };

    // Ejecuta un quantum pero permite un 'tickHook' invocado en cada tick.
    // Si 'tickHook' retorna true, el proceso será bloqueado (estado BLOCKED)
    // y no se reencolará. El hook recibe (Process*, tickIndex).
    QuantumResult runQuantumDetailed(const std::function<bool(Process *, int)> &tickHook = nullptr);

    // Ejecuta el ciclo completo Round-Robin hasta vaciar la cola
    void runAll();

    // Imprime el estado actual de la cola de listos
    void printQueue() const;

    // Getters de diagnóstico
    bool   isEmpty()    const { return readyQueue_.empty(); }
    int    getClock()   const { return clock_; }
    int    getQuantum() const { return quantum_; }
};

#endif