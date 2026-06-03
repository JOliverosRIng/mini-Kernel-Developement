#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <string>
#include <map>
#include <vector>

static const int MAX_FILE_SIZE = 512;
static const int MAX_FILES = 32;

// File Control Block — metadata de archivo
struct FCB
{
    std::string name;
    int size;
    int createdBy;
    std::string createdByName;
    bool isOpen;
    int openedBy; // PID del proceso que lo tiene abierto (-1 si cerrado)
    std::string openedByName;
};

class FileSystem
{
private:
    std::map<std::string, FCB> directory_; // Directorio raíz (nombre -> FCB)
    int fileCount_;

public:
    FileSystem();
    bool create(const std::string &name, int size, int pid, const std::string &processName = "");
    bool open(const std::string &name, int pid, const std::string &processName = "");
    bool close(const std::string &name, int pid, const std::string &processName = "");
    bool remove(const std::string &name, int pid, const std::string &processName = "");
    bool exists(const std::string &name) const;
    void printDirectory() const;
};

#endif
