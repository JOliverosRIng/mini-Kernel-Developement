#include "Scheduler.hpp"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

Scheduler::Scheduler(int quantum)
    : quantum_(quantum), clock_(0)
{
    std::cout << "[Scheduler] Inicializado - quantum=" << quantum_ << " ticks\n";
}

void Scheduler::printSection(const std::string &title) const
{
    std::cout << "\n" << title << ":\n";
}

void Scheduler::enqueue(Process *p)
{
    if (p->getState() == ProcessState::TERMINATED)
    {
        std::cout << "[Scheduler] aviso: proceso '" << p->getName() << "'"
                  << " ya esta terminado, no se encola.\n";
        return;
    }

    p->setState(ProcessState::READY);
    readyQueue_.push(p);

    std::cout << "[Scheduler] proceso '" << p->getName() << "' -> listo "
              << "(" << readyQueue_.size() << " en cola)\n";
}

void Scheduler::unblock(Process *p)
{
    auto it = std::find(blocked_.begin(), blocked_.end(), p);
    if (it != blocked_.end())
        blocked_.erase(it);

    std::cout << "[Scheduler] Proceso '" << p->getName() << "'"
              << " desbloqueado -> listo\n";
    enqueue(p);
}

bool Scheduler::runQuantum()
{
    QuantumResult r = runQuantumDetailed(nullptr);
    (void)r;
    return !readyQueue_.empty();
}

Scheduler::QuantumResult Scheduler::runQuantumDetailed(const std::function<bool(Process *, int)> &tickHook)
{
    QuantumResult result{nullptr, false, 0};

    if (readyQueue_.empty())
        return result;

    Process *p = readyQueue_.front();
    readyQueue_.pop();
    result.proc = p;

    p->setState(ProcessState::RUNNING);

    std::cout << "\n[t=" << std::setw(3) << clock_ << "] "
              << "ejecutando: '" << p->getName() << "'"
              << "  tiempo restante=" << p->getRemainingTime() << "\n";

    int ticksEjecutados = 0;

    while (ticksEjecutados < quantum_ && p->getRemainingTime() > 0)
    {
        bool terminado = p->decrementTime();
        ++clock_;
        ++ticksEjecutados;

        std::this_thread::sleep_for(std::chrono::milliseconds(250));

        std::cout << "  [t=" << std::setw(3) << clock_ << "]"
                  << "  tick #" << ticksEjecutados
                  << "  tiempo restante=" << p->getRemainingTime();

        if (tickHook && tickHook(p, ticksEjecutados))
        {
            p->setState(ProcessState::BLOCKED);
            blocked_.push_back(p);
            std::cout << " (bloqueado por e/s)\n";
            break;
        }

        if (terminado)
        {
            std::cout << " (terminado)\n";
            break;
        }
        std::cout << "\n";
    }

    result.ticksExecuted = ticksEjecutados;

    if (p->getRemainingTime() == 0)
    {
        p->setState(ProcessState::TERMINATED);
        std::cout << "[Scheduler] proceso '" << p->getName() << "'"
                  << " -> terminado [t=" << clock_ << "]\n";
        result.terminated = true;
    }
    else if (p->getState() == ProcessState::BLOCKED)
    {
        std::cout << "[Scheduler] proceso '" << p->getName() << "'"
                  << " bloqueado, queda: " << p->getRemainingTime() << "\n";
    }
    else
    {
        p->setState(ProcessState::READY);
        readyQueue_.push(p);
        std::cout << "[Scheduler] proceso '" << p->getName() << "'"
                  << " -> listo (vuelve a cola), queda: " 
                  << p->getRemainingTime() << "\n";
    }

    return result;
}

void Scheduler::runAll()
{
    printSection("planificador round-robin - inicio");
    std::cout << "quantum=" << quantum_
              << "  procesos en cola: " << readyQueue_.size() << "\n";

    std::cout << "\n"
              << std::setw(8) << "tick"
              << std::setw(18) << "proceso"
              << std::setw(14) << "estado"
              << "restante\n";

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
                      << std::setw(8) << clock_
                      << std::setw(18) << p->getName()
                      << std::setw(14) << "ejecutando"
                      << std::setw(18) << p->getRemainingTime()
                      << "\n";

            if (terminado)
                break;
        }

        if (p->getRemainingTime() == 0)
        {
            p->setState(ProcessState::TERMINATED);
            std::cout << std::left
                      << std::setw(8) << clock_
                      << std::setw(18) << p->getName()
                      << std::setw(14) << "terminado"
                      << std::setw(18) << 0
                      << "\n";
        }
        else
        {
            p->setState(ProcessState::READY);
            readyQueue_.push(p);
        }
    }

    printSection("planificador - fin");
    std::cout << "tiempo total simulado: " << clock_ << " ticks\n\n";
}

void Scheduler::printQueue() const
{
    std::cout << "\ncola de listos:\n";
    std::cout << std::left
              << std::left << std::setw(18) << "proceso"
              << std::setw(14) << "estado"
              << "restante\n";

    std::queue<Process *> copia = readyQueue_;
    if (copia.empty())
    {
        std::cout << " (vacia)\n";
    }
    while (!copia.empty())
    {
        Process *p = copia.front();
        copia.pop();
        std::cout << std::left
                  << std::setw(18) << p->getName()
                  << std::setw(14) << "listo"
                  << std::setw(18) << p->getRemainingTime() << "\n";
    }
}
