#include "Kernel.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

namespace
{
void demoPause(int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
} // namespace

Kernel::Kernel(int quantum, std::function<void(const std::string &)> logger)
    : mem_(), paging_(), sched_(quantum), fs_(), io_(), nextPid_(1), logger_(logger)
{
    log("Kernel inicializado");
}

Kernel::~Kernel()
{
    for (auto &p : processes_)
        delete p.second;
}

void Kernel::log(const std::string &s) const
{
    if (logger_)
        logger_(s);
    else
        std::cout << s << std::endl;
}

Process *Kernel::createProcess(const std::string &name, int burstTime, int memSize, int ioAtTick)
{
    // Intentamos asignar memoria antes de crear el objeto Process formalmente
    int base = mem_.allocate(nextPid_, memSize, name);
    if (base != -1)
    {
        int pid = nextPid_++;
        Process *p = new Process(pid, name, burstTime, memSize);
        p->setMemoryStart(base);
        paging_.createTable(pid, base, memSize, name);
        
        processes_[pid] = p;
        if (ioAtTick >= 1 && ioAtTick < burstTime)
        {
            ioAtTick_[pid] = ioAtTick;
        }

        sched_.enqueue(p);

        log("[Kernel] Recursos listos para el proceso '" + name + "': memoria reservada y tabla de paginas creada");
        log("[Kernel] Proceso creado: '" + name + "' tiempo_CPU=" + std::to_string(burstTime) + 
            " memoria=" + std::to_string(memSize));
        return p;
    }
    
    // Si no hay memoria, no creamos nada y retornamos nullptr
    return nullptr;
}

void Kernel::tryLoadCpuJobs(std::vector<CpuProcessSpec> &pending)
{
    while (!pending.empty())
    {
        auto &s = pending.front();
        if (createProcess(s.name, s.burstTime, s.memSize))
            pending.erase(pending.begin());
        else
            break;
    }
}

void Kernel::tryLoadIoJobs(std::vector<IoProcessSpec> &pending)
{
    while (!pending.empty())
    {
        auto &s = pending.front();
        if (createProcess(s.name, s.burstTime, s.memSize, s.ioAtTick))
            pending.erase(pending.begin());
        else
            break;
    }
}

void Kernel::handleProcessTermination(Process *p)
{
    int pid = p->getPid();
    const std::string name = p->getName();
    mem_.free(pid, name);
    paging_.removeTable(pid, name);
    log("[Kernel] El proceso '" + name + "' finalizo y sus recursos fueron liberados.");
}

void Kernel::runScenarioCPUAndMemoryContention(const std::vector<CpuProcessSpec> &specs)
{
    log("[Scenario] CPU y memoria: se inicia la simulacion de competencia por recursos.");
    log("[Scenario] Entrada de usuario: se leera la lista de procesos del panel.");

    demoPause(800);
    log("[Scenario] Paso 1: el kernel crea procesos con distintas cargas de trabajo.");

    std::vector<CpuProcessSpec> pending = specs;
    tryLoadCpuJobs(pending);

    demoPause(1000);
    log("[Scenario] Paso 2: el planificador Round-Robin toma el control.");

    while (!sched_.isEmpty() || !pending.empty())
    {
        demoPause(700);
        log("[Scenario] Se despacha el siguiente proceso listo para ejecutar un quantum.");
        Scheduler::QuantumResult r = sched_.runQuantumDetailed(nullptr);

        if (r.terminated && r.proc)
        {
            handleProcessTermination(r.proc);
            tryLoadCpuJobs(pending);
        }
    }

    log("[Scenario] Resultado final: todos los procesos terminaron y la competencia por CPU quedo resuelta.");
}

void Kernel::runScenarioSimultaneousIO(const std::vector<IoProcessSpec> &specs)
{
    log("[Scenario] E/S simultanea: se inicia la simulacion de bloqueos y reanudaciones.");
    log("[Scenario] Entrada de usuario: se leera la lista de procesos y su punto de E/S.");

    demoPause(800);
    log("[Scenario] Paso 1: el kernel prepara procesos que solicitaran E/S durante su ejecucion.");

    std::vector<IoProcessSpec> pending = specs;
    tryLoadIoJobs(pending);

    demoPause(1000);

    auto hook = [this](Process *proc, int tickIdx) -> bool {
        int pid = proc->getPid();
        const std::string &name = proc->getName();
        auto it = ioAtTick_.find(pid);
        if (it != ioAtTick_.end() && it->second == tickIdx)
        {
            std::string fname = "file_" + name + ".dat";
            if (!fs_.exists(fname))
                fs_.create(fname, 64, pid, name);

            io_.requestIO(pid, IOType::READ, fname, name);
            log("[Kernel] El proceso '" + name + "' solicito E/S sobre '" + fname + "' y pasa a BLOCKED.");
            return true;
        }
        return false;
    };

    demoPause(1200);
    log("[Scenario] Paso 2: la CPU comienza a ejecutar y el sistema atiende la cola de E/S en paralelo.");

    while (!sched_.isEmpty() || io_.isBusy() || io_.queueSize() > 0 || !pending.empty())
    {
        demoPause(600);
        int ioSteps = 0;

        if (!sched_.isEmpty())
        {
            Scheduler::QuantumResult r = sched_.runQuantumDetailed(hook);
            ioSteps = std::max(1, r.ticksExecuted);
            
            if (r.terminated && r.proc)
            {
                handleProcessTermination(r.proc);
                tryLoadIoJobs(pending);
            }
        }
        else
        {
            log("[Kernel] CPU ociosa - esperando e/s...");
            ioSteps = 1;
        }

        for (int i = 0; i < ioSteps; ++i)
        {
            int completedPid = io_.processAndGetCompletedPid();
            if (completedPid > 0)
            {
                auto it = processes_.find(completedPid);
                if (it != processes_.end())
                {
                    sched_.unblock(it->second);
                    log("[Kernel] La E/S termino para el proceso '" + it->second->getName() + 
                        "' y regresa a la cola de listos.");
                }
            }
        }
    }

    log("[Scenario] Resultado final: la cola de E/S se vacio y todos los procesos completaron su trabajo.");
}

void Kernel::runScenarioCPUAndMemoryContention()
{
    std::vector<CpuProcessSpec> specs = {
        {"P_A", 10, 400},
        {"P_B", 8, 400},
        {"P_C", 6, 300},
        {"P_D", 12, 200},
    };
    runScenarioCPUAndMemoryContention(specs);
}

void Kernel::runScenarioSimultaneousIO()
{
    std::vector<IoProcessSpec> specs = {
        {"IO_1", 9, 100, 2},
        {"IO_2", 9, 100, 2},
        {"IO_3", 6, 100, 3},
    };
    runScenarioSimultaneousIO(specs);
}
