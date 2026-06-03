#include "filesystem.hpp"
#include "io.hpp"
#include <iostream>

static int passed = 0;
static int failed = 0;

#define TEST(name, expr)                        \
    do                                          \
    {                                           \
        if (expr)                               \
        {                                       \
            std::cout << "  [PASS] " name "\n"; \
            ++passed;                           \
        }                                       \
        else                                    \
        {                                       \
            std::cout << "  [FAIL] " name "\n"; \
            ++failed;                           \
        }                                       \
    } while (false)

static void testFileSystemCreate()
{
    std::cout << "\n[TEST] FileSystem::create\n";
    FileSystem fs;

    bool r1 = fs.create("archivo1.txt", 100, 1);
    TEST("Create valid file succeeds", r1 == true);

    bool r2 = fs.create("archivo2.txt", 256, 2);
    TEST("Create second file succeeds", r2 == true);

    bool r3 = fs.create("archivo1.txt", 50, 1);
    TEST("Create duplicate file fails", r3 == false);

    bool r4 = fs.create("", 100, 1);
    TEST("Create empty name fails", r4 == false);

    bool r5 = fs.create("invalid.txt", -10, 1);
    TEST("Create negative size fails", r5 == false);

    bool r6 = fs.create("toolarge.txt", MAX_FILE_SIZE + 1, 1);
    TEST("Create size > MAX_FILE_SIZE fails", r6 == false);

    fs.printDirectory();
}

static void testFileSystemOpenClose()
{
    std::cout << "\n[TEST] FileSystem::open + close\n";
    FileSystem fs;

    fs.create("test.txt", 100, 1);

    bool r1 = fs.open("test.txt", 1);
    TEST("Open existing file succeeds", r1 == true);

    bool r2 = fs.open("test.txt", 2);
    TEST("Open already opened file fails", r2 == false);

    bool r3 = fs.close("test.txt", 2);
    TEST("Close by wrong PID fails", r3 == false);

    bool r4 = fs.close("test.txt", 1);
    TEST("Close by correct PID succeeds", r4 == true);

    bool r5 = fs.close("test.txt", 1);
    TEST("Close already closed file fails", r5 == false);

    bool r6 = fs.open("noexiste.txt", 1);
    TEST("Open non-existent file fails", r6 == false);

    fs.printDirectory();
}

static void testFileSystemRemove()
{
    std::cout << "\n[TEST] FileSystem::remove\n";
    FileSystem fs;

    fs.create("temp.txt", 50, 1);
    fs.open("temp.txt", 1);

    bool r1 = fs.remove("temp.txt", 1);
    TEST("Remove opened file fails", r1 == false);

    fs.close("temp.txt", 1);

    bool r2 = fs.remove("temp.txt", 1);
    TEST("Remove closed file succeeds", r2 == true);

    bool r3 = fs.exists("temp.txt");
    TEST("Removed file no longer exists", r3 == false);

    bool r4 = fs.remove("noexiste.txt", 1);
    TEST("Remove non-existent file fails", r4 == false);

    fs.printDirectory();
}

static void testFileSystemExists()
{
    std::cout << "\n[TEST] FileSystem::exists\n";
    FileSystem fs;

    fs.create("existe.txt", 100, 1);

    bool r1 = fs.exists("existe.txt");
    TEST("Existing file returns true", r1 == true);

    bool r2 = fs.exists("noexiste.txt");
    TEST("Non-existing file returns false", r2 == false);

    fs.remove("existe.txt", 1);

    bool r3 = fs.exists("existe.txt");
    TEST("Removed file returns false", r3 == false);
}

static void testIOManagerQueue()
{
    std::cout << "\n[TEST] IOManager::requestIO + queueSize\n";
    IOManager io;

    TEST("Initially queue is empty", io.queueSize() == 0);
    TEST("Initially not busy", io.isBusy() == false);

    io.requestIO(1, IOType::READ, "archivo1.txt");
    TEST("After request queue size = 1", io.queueSize() == 1);

    io.requestIO(2, IOType::WRITE, "archivo2.txt");
    io.requestIO(3, IOType::READ, "archivo3.txt");
    TEST("After 3 requests queue size = 3", io.queueSize() == 3);

    io.printQueue();
}

static void testIOManagerProcess()
{
    std::cout << "\n[TEST] IOManager::processIO (ciclo completo)\n";
    IOManager io;

    io.requestIO(1, IOType::READ, "test.txt");

    // Primer processIO toma de la cola y comienza (ciclo 1 de 3)
    bool r1 = io.processIO();
    TEST("Cycle 1 of 3 not completed yet", r1 == false);
    TEST("After starting, queue is empty", io.queueSize() == 0);
    TEST("Device is now busy", io.isBusy() == true);

    // Ciclo 2 de 3
    bool r2 = io.processIO();
    TEST("Cycle 2 of 3 not completed yet", r2 == false);
    TEST("Still busy", io.isBusy() == true);

    // Ciclo 3 de 3 (completa)
    bool r3 = io.processIO();
    TEST("Cycle 3 of 3 completes (returns true)", r3 == true);
    TEST("Device is now free", io.isBusy() == false);

    // Intentar procesar sin solicitudes
    bool r4 = io.processIO();
    TEST("Process with empty queue returns false", r4 == false);

    io.printStatus();
}

static void testIOManagerMultipleRequests()
{
    std::cout << "\n[TEST] IOManager::processIO (multiples operaciones)\n";
    IOManager io;

    io.requestIO(1, IOType::READ, "file1.txt");
    io.requestIO(2, IOType::WRITE, "file2.txt");
    io.requestIO(3, IOType::OPEN, "file3.txt");

    // Procesar primera operación (3 ciclos)
    io.processIO();                   // Ciclo 1
    io.processIO();                   // Ciclo 2
    bool completed1 = io.processIO(); // Ciclo 3 - completa
    TEST("First operation completes", completed1 == true);
    TEST("Queue has 2 remaining", io.queueSize() == 2);

    // Procesar segunda operación (3 ciclos)
    io.processIO();                   // Ciclo 1
    io.processIO();                   // Ciclo 2
    bool completed2 = io.processIO(); // Ciclo 3 - completa
    TEST("Second operation completes", completed2 == true);
    TEST("Queue has 1 remaining", io.queueSize() == 1);

    // Procesar tercera operación
    io.processIO();
    io.processIO();
    bool completed3 = io.processIO();
    TEST("Third operation completes", completed3 == true);
    TEST("Queue is now empty", io.queueSize() == 0);

    io.printStatus();
}

int main()
{
    std::cout << "╔═══════════════════════════════════════════╗\n";
    std::cout << "║   TEST SUITE — Fase 3: FileSystem y E/S ║\n";
    std::cout << "╚═══════════════════════════════════════════╝\n";

    // Tests de FileSystem
    testFileSystemCreate();
    testFileSystemOpenClose();
    testFileSystemRemove();
    testFileSystemExists();

    // Tests de IOManager
    testIOManagerQueue();
    testIOManagerProcess();
    testIOManagerMultipleRequests();

    // Resumen
    std::cout << "\n─────────────────────────────────────────────\n";
    std::cout << "Resultados: " << passed << " pasaron, " << failed << " fallaron.\n";

    return (failed > 0) ? 1 : 0;
}
