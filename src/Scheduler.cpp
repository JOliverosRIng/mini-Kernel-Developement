#include "Scheduler.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>

// Constructor
Scheduler::Scheduler(int quantum)
    : quantum_(quantum), clock_(0)
{
    std::cout << "[Scheduler] Inicializado — quantum=" << quantum_ << " ticks\n";
}

// printSection
//es una función auxiliar para imprimir títulos de sección con formato
void Scheduler::printSection(const std::string &title) const
{
    const int W = 52;
    std::cout << "\n+" << std::string(W, '-') << "+\n";
    std::cout << "|  " << std::left << std::setw(W - 2) << title << "|\n";
    std::cout << "+" << std::string(W, '-') << "+\n";
}

// enqueue - Cola de listos
// NEW READY: el proceso entra a la cola de listos
void Scheduler::enqueue(Process *p)
{
    if (p->getState() == ProcessState::TERMINATED)
    {
        std::cout << "[Scheduler] WARN: PID=" << p->getPid()
                  << " ya está TERMINATED, no se encola.\n";
        return;
    }

    p->setState(ProcessState::READY);
    readyQueue_.push(p);

    std::cout << "[Scheduler] PID=" << p->getPid()
              << " (" << p->getName() << ") → READY  "
              << "[Cola: " << readyQueue_.size() << " proceso(s)]\n";
}

// unblock - bloqueo de E/S
// BLOCKED READY: el proceso vuelve de una espera de E/S
void Scheduler::unblock(Process *p)
{
    // Quitar de la lista de bloqueados
    auto it = std::find(blocked_.begin(), blocked_.end(), p);
    if (it != blocked_.end())
        blocked_.erase(it);

    std::cout << "[Scheduler] PID=" << p->getPid()
              << " BLOCKED → READY (desbloqueado)\n";
    enqueue(p);
}

// runQuantum - Ejecución de un quantum
// Ejecuta UN quantum sobre el proceso al frente de la cola
bool Scheduler::runQuantum()
{
    if (readyQueue_.empty())
        return false;

    // Sacar el proceso al frente
    Process *p = readyQueue_.front();
    readyQueue_.pop();

    // READY: RUNNING
    p->setState(ProcessState::RUNNING);

    std::cout << "\n[t=" << std::setw(3) << clock_ << "] "
              << "RUNNING  PID=" << p->getPid()
              << " (" << p->getName() << ")"
              << "  rem=" << p->getRemainingTime() << "\n";

    // Ejecutar tick a tick hasta agotar el quantum o terminar
    int ticksEjecutados = 0;

    while (ticksEjecutados < quantum_ && p->getRemainingTime() > 0)
    {
        bool terminado = p->decrementTime(); // baja 1 tick
        ++clock_;
        ++ticksEjecutados;

        std::cout << "  [t=" << std::setw(3) << clock_ << "]"
                  << "  tick #" << ticksEjecutados
                  << "  rem=" << p->getRemainingTime();

        if (terminado)
        {
            std::cout << "  ← TERMINADO\n";
            break;
        }
        std::cout << "\n";
    }

    // Decidir transición de estado al terminar el quantum
    if (p->getRemainingTime() == 0)
    {
        // RUNNING → TERMINATED
        p->setState(ProcessState::TERMINATED);
        std::cout << "[Scheduler] PID=" << p->getPid()
                  << " → TERMINATED  [t=" << clock_ << "]\n";
    }
    else
    {
        // RUNNING → READY: reencolar al final (Round-Robin)
        p->setState(ProcessState::READY);
        readyQueue_.push(p);
        std::cout << "[Scheduler] PID=" << p->getPid()
                  << " → READY (quantum agotado, vuelve a la cola)"
                  << "  rem=" << p->getRemainingTime() << "\n";
    }

    return !readyQueue_.empty();
}

// ── runAll ────────────────────────────────────────────────────
// Ciclo completo Round-Robin hasta que todos los procesos terminen
void Scheduler::runAll()
{
    printSection("PLANIFICADOR ROUND-ROBIN — inicio");
    std::cout << "Quantum=" << quantum_
              << "  Procesos en cola: " << readyQueue_.size() << "\n";

    // Encabezado de tabla de traza
    std::cout << "\n"
              << std::left
              << std::setw(8)  << "Tick"
              << std::setw(8)  << "PID"
              << std::setw(16) << "Nombre"
              << std::setw(14) << "Estado"
              << std::setw(10) << "Restante"
              << "\n"
              << std::string(56, '-') << "\n";

    while (!readyQueue_.empty())
    {
        Process *p = readyQueue_.front();
        readyQueue_.pop();

        p->setState(ProcessState::RUNNING);

        int ticksEjecutados = 0;

        while (ticksEjecutados < quantum_ && p->getRemainingTime() > 0)
        {
            bool terminado = p->decrementTime();
            ++clock_;
            ++ticksEjecutados;

            std::cout << std::left
                      << std::setw(8)  << clock_
                      << std::setw(8)  << p->getPid()
                      << std::setw(16) << p->getName()
                      << std::setw(14) << "RUNNING"
                      << std::setw(10) << p->getRemainingTime()
                      << "\n";

            if (terminado) break;
        }

        if (p->getRemainingTime() == 0)
        {
            p->setState(ProcessState::TERMINATED);
            std::cout << std::left
                      << std::setw(8)  << clock_
                      << std::setw(8)  << p->getPid()
                      << std::setw(16) << p->getName()
                      << std::setw(14) << "TERMINATED"
                      << std::setw(10) << 0
                      << "\n";
        }
        else
        {
            p->setState(ProcessState::READY);
            readyQueue_.push(p);
        }
    }

    printSection("PLANIFICADOR — fin");
    std::cout << "Tiempo total simulado: " << clock_ << " ticks\n\n";
}

// ── printQueue ────────────────────────────────────────────────
void Scheduler::printQueue() const
{
    const int W = 52;
    std::cout << "\n+" << std::string(W, '=') << "+\n";
    std::cout << "|  COLA DE LISTOS"
              << std::string(W - 16, ' ') << "|\n";
    std::cout << "+" << std::string(W, '=') << "+\n";
    std::cout << "| "
              << std::left << std::setw(6)  << "PID"
              << std::setw(16) << "Nombre"
              << std::setw(12) << "Estado"
              << std::setw(10) << "Restante"
              << "  |\n";
    std::cout << "+" << std::string(W, '-') << "+\n";

    // std::queue no permite iteración directa, copiamos para recorrer
    std::queue<Process *> copia = readyQueue_;
    if (copia.empty())
    {
        std::cout << "|  (cola vacía)"
                  << std::string(W - 14, ' ') << "|\n";
    }
    while (!copia.empty())
    {
        Process *p = copia.front();
        copia.pop();
        std::cout << "| "
                  << std::setw(6)  << p->getPid()
                  << std::setw(16) << p->getName()
                  << std::setw(12) << "READY"
                  << std::setw(10) << p->getRemainingTime()
                  << "  |\n";
    }
    std::cout << "+" << std::string(W, '=') << "+\n\n";
}