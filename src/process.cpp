#include "process.hpp"
#include <sstream>

Process::Process(int pid, const std::string &name, int burstTime, int memSize)
    : pid_(pid), name_(name), state_(ProcessState::NEW), burstTime_(burstTime), remainingTime_(burstTime), memoryStart_(-1), memorySize_(memSize)
{
}

int Process::getPid() const { return pid_; }
std::string Process::getName() const { return name_; }
ProcessState Process::getState() const { return state_; }
int Process::getRemainingTime() const { return remainingTime_; }
int Process::getMemoryStart() const { return memoryStart_; }
int Process::getMemorySize() const { return memorySize_; }

void Process::setState(ProcessState s) { state_ = s; }
void Process::setMemoryStart(int addr) { memoryStart_ = addr; }

bool Process::decrementTime()
{
    if (remainingTime_ > 0)
    {
        --remainingTime_;
    }
    return remainingTime_ == 0;
}

static const char *stateStr(ProcessState s)
{
    switch (s)
    {
    case ProcessState::NEW:
        return "NEW";
    case ProcessState::READY:
        return "READY";
    case ProcessState::RUNNING:
        return "RUNNING";
    case ProcessState::BLOCKED:
        return "BLOCKED";
    case ProcessState::TERMINATED:
        return "TERMINATED";
    }
    return "?";
}

std::string Process::toString() const
{
    std::ostringstream oss;
    oss << "[PID=" << pid_
        << " name=" << name_
        << " state=" << stateStr(state_)
        << " rem=" << remainingTime_
        << " memBase=" << memoryStart_
        << " memSz=" << memorySize_ << "]";
    return oss.str();
}