#ifndef KERNEL_HPP
#define KERNEL_HPP

#include "memory.hpp"
#include "paging.hpp"
#include "process.hpp"
#include "Scheduler.hpp"
#include "filesystem.hpp"
#include "io.hpp"

#include <map>
#include <functional>
#include <string>
#include <vector>

class Kernel
{
public:
    struct CpuProcessSpec
    {
        std::string name;
        int burstTime;
        int memSize;
    };

    struct IoProcessSpec
    {
        std::string name;
        int burstTime;
        int memSize;
        int ioAtTick;
    };

private:
    MemoryManager mem_;
    PagingUnit paging_;
    Scheduler sched_;
    FileSystem fs_;
    IOManager io_;

    std::map<int, Process *> processes_;
    std::map<int, int> ioAtTick_; // pid -> tick within quantum to request IO
    int nextPid_;

    std::function<void(const std::string &)> logger_;

    void log(const std::string &s) const;

    // Auxiliares para gestionar colas de trabajos pendientes
    void tryLoadCpuJobs(std::vector<CpuProcessSpec> &pending);
    void tryLoadIoJobs(std::vector<IoProcessSpec> &pending);
    void handleProcessTermination(Process *p);

public:
    explicit Kernel(int quantum = 3, std::function<void(const std::string &)> logger = nullptr);
    ~Kernel();

    // Crear proceso y asignar memoria + tabla de paginas
    Process *createProcess(const std::string &name, int burstTime, int memSize, int ioAtTick = -1);

    // Escenarios de prueba integrados
    void runScenarioCPUAndMemoryContention(const std::vector<CpuProcessSpec> &specs);
    void runScenarioSimultaneousIO(const std::vector<IoProcessSpec> &specs);
    void runScenarioCPUAndMemoryContention();
    void runScenarioSimultaneousIO();
};

#endif
