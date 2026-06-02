#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <string>

constexpr int MAX_PROCESSES = 10;

enum class ProcessState
{
    NEW,
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
};

class Process
{
public:
    Process(int pid, const std::string &name, int burstTime, int memSize);

    int getPid() const;
    std::string getName() const;
    ProcessState getState() const;
    int getRemainingTime() const;
    int getMemoryStart() const;
    int getMemorySize() const;

    void setState(ProcessState s);
    void setMemoryStart(int addr);

    bool decrementTime();

    std::string toString() const;

private:
    int pid_;
    std::string name_;
    ProcessState state_;
    int burstTime_;
    int remainingTime_;
    int memoryStart_;
    int memorySize_;
};

#endif
