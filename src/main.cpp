#include "memory.hpp"
#include "paging.hpp"
#include "process.hpp"
#include "Scheduler.hpp" 
#include "filesystem.hpp"
#include "io.hpp"
#include <iostream>
#include <limits>

static void pausa(const char *mensaje = "Presione Enter para continuar...")
{
    std::cout << "\n>>> " << mensaje << " ";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static void titulo(const char *texto)
{
    const int ancho = 60;
    std::cout << "\n"
              << std::string(ancho, '=') << "\n";
    std::cout << "  " << texto << "\n";
    std::cout << std::string(ancho, '=') << "\n\n";
}

static void seccion(const char *texto)
{
    std::cout << "\n--- " << texto << " ---\n\n";
}

static void limpiarBuffer()
{
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static void demoFase1()
{
    titulo("FASE 1: Gestion de Procesos (Round-Robin)");

    // Crear procesos
    // Constructor de tu Process: (pid, name, burstTime, memSize)
    Process p1(1, "Editor",      9, 200);
    Process p2(2, "Compilador",  6, 150);
    Process p3(3, "Player",      4, 100);
    Process p4(4, "Browser",     7, 180);

    seccion("Estado inicial — todos en NEW");
    std::cout << p1.toString() << "\n";
    std::cout << p2.toString() << "\n";
    std::cout << p3.toString() << "\n";
    std::cout << p4.toString() << "\n";
    pausa();

    // Encolar: NEW a READY
    Scheduler sched(3); // quantum = 3 ticks
    seccion("Encolando procesos: NEW → READY");
    sched.enqueue(&p1);
    sched.enqueue(&p2);
    sched.enqueue(&p3);
    sched.enqueue(&p4);
    sched.printQueue();
    pausa();

    // Round-Robin paso a paso
    seccion("Planificacion Round-Robin — turno a turno");
    std::cout << "Se ejecuta un quantum (3 ticks) por turno.\n"
              << "Si el proceso no termina, vuelve al final de la cola.\n\n";

    int turno = 1;
    while (!sched.isEmpty())
    {
        std::cout << "\n── Turno " << turno++ << " ──\n";
        sched.runQuantum();
    }
    pausa();

    // Estado final
    seccion("Estado final — todos TERMINATED");
    std::cout << p1.toString() << "\n";
    std::cout << p2.toString() << "\n";
    std::cout << p3.toString() << "\n";
    std::cout << p4.toString() << "\n";

    std::cout << "\nTiempo total simulado: " << sched.getClock() << " ticks\n";
    pausa("Presione Enter para volver al menu principal");
}

static void demoFase2()
{
    titulo("FASE 2: Gestion de Memoria (First-Fit + Paginacion)");

    MemoryManager mem;
    PagingUnit paging;

    seccion("Estado inicial de la memoria (1024 unidades libres)");
    mem.printMap();
    pausa();

    seccion("Asignando memoria a 3 procesos (algoritmo First-Fit)");
    std::cout << "- PID 1: 200 unidades\n";
    std::cout << "- PID 2: 300 unidades\n";
    std::cout << "- PID 3: 128 unidades\n\n";

    int a1 = mem.allocate(1, 200);
    int a2 = mem.allocate(2, 300);
    int a3 = mem.allocate(3, 128);
    mem.printMap();
    pausa();

    seccion("Creando tablas de paginas (PAGE_SIZE=64, 16 marcos)");
    if (a1 != -1)
    {
        paging.createTable(1, a1, 200);
        paging.printTable(1);
    }
    pausa("Presione Enter para ver la tabla del Proceso 2");
    if (a2 != -1)
    {
        paging.createTable(2, a2, 300);
        paging.printTable(2);
    }
    pausa("Presione Enter para ver la tabla del Proceso 3");
    if (a3 != -1)
    {
        paging.createTable(3, a3, 128);
        paging.printTable(3);
    }
    pausa();

    seccion("Traduccion de direcciones logicas a fisicas (Proceso 2)");
    std::cout << "Direccion logica 0   -> primera pagina, offset 0\n";
    paging.translate(2, 0);
    std::cout << "\nDireccion logica 64  -> segunda pagina, offset 0\n";
    paging.translate(2, 64);
    std::cout << "\nDireccion logica 127 -> segunda pagina, offset 63\n";
    paging.translate(2, 127);
    std::cout << "\nDireccion logica 700 -> FUERA DE RANGO (fallo de pagina)\n";
    paging.translate(2, 700);
    pausa();

    seccion("Liberando PID 2 -> se crea un hueco (fragmentacion)");
    mem.free(2);
    paging.removeTable(2);
    mem.printMap();
    pausa();

    seccion("Intentando asignar 350 unidades (fragmentacion externa)");
    std::cout << "Total libre: 724 unidades, pero no hay bloque contiguo >= 350\n\n";
    mem.allocate(4, 350);
    pausa();

    seccion("Liberando PID 1 -> COALESCING fusiona bloques adyacentes");
    mem.free(1);
    paging.removeTable(1);
    mem.printMap();
    std::cout << "RESULTADO: Los dos huecos contiguos se fusionaron en uno de 500 unidades\n";
    pausa();

    seccion("Ahora SI se puede asignar el bloque de 350 unidades");
    int a4 = mem.allocate(4, 350);
    if (a4 != -1)
        paging.createTable(4, a4, 350);
    mem.printMap();
    pausa("Presione Enter para volver al menu principal");
}

static void demoFase3()
{
    titulo("FASE 3: Sistema de Archivos y E/S");

    FileSystem fs;
    IOManager io;

    seccion("Estado inicial del sistema de archivos");
    fs.printDirectory();
    pausa();

    seccion("Creando archivos en el sistema (PID 1)");
    std::cout << "Creando 3 archivos:\n";
    std::cout << "- datos.txt (256 bytes)\n";
    std::cout << "- config.ini (128 bytes)\n";
    std::cout << "- log.txt (64 bytes)\n\n";

    fs.create("datos.txt", 256, 1);
    fs.create("config.ini", 128, 1);
    fs.create("log.txt", 64, 1);
    fs.printDirectory();
    pausa();

    seccion("Abriendo archivo para lectura (PID 2)");
    std::cout << "PID 2 intenta abrir 'datos.txt'...\n\n";
    fs.open("datos.txt", 2);
    fs.printDirectory();
    pausa();

    seccion("Solicitando operaciones de E/S");
    std::cout << "PID 2 solicita READ de 'datos.txt'\n";
    std::cout << "PID 1 solicita WRITE de 'config.ini'\n";
    std::cout << "PID 3 solicita READ de 'log.txt'\n\n";

    io.requestIO(2, IOType::READ, "datos.txt");
    io.requestIO(1, IOType::WRITE, "config.ini");
    io.requestIO(3, IOType::READ, "log.txt");
    io.printQueue();
    pausa();

    seccion("Procesando E/S - Ciclo 1 (latencia: 3 ciclos)");
    io.processIO();
    io.printStatus();
    pausa("Presione Enter para siguiente ciclo");

    seccion("Procesando E/S - Ciclo 2");
    io.processIO();
    io.printStatus();
    pausa("Presione Enter para siguiente ciclo");

    seccion("Procesando E/S - Ciclo 3 (interrupcion)");
    std::cout << "Al completarse, se genera una INTERRUPCION de E/S\n\n";
    io.processIO();
    io.printQueue();
    pausa();

    seccion("Continuando con la siguiente operacion en cola");
    io.processIO(); // Ciclo 1 de WRITE
    io.processIO(); // Ciclo 2 de WRITE
    io.processIO(); // Ciclo 3 de WRITE (completa)
    io.printQueue();
    pausa();

    seccion("Cerrando archivo (PID 2)");
    std::cout << "PID 2 cierra 'datos.txt'...\n\n";
    fs.close("datos.txt", 2);
    fs.printDirectory();
    pausa();

    seccion("Intentando eliminar archivo abierto (validacion)");
    std::cout << "PID 1 intenta eliminar 'config.ini' (deberia fallar)...\n\n";
    fs.remove("config.ini", 1);
    pausa();

    seccion("Cerrando y eliminando archivo correctamente");
    std::cout << "PID 3 cierra 'log.txt' y luego lo elimina...\n\n";
    fs.open("log.txt", 3);
    fs.close("log.txt", 3);
    fs.remove("log.txt", 3);
    fs.printDirectory();

    pausa("Presione Enter para volver al menu principal");
}

static void demoCompleta()
{
    titulo("DEMOSTRACION COMPLETA: Kernel Integrado");
    std::cout << "\nEn desarrollo...\n\n";
    pausa("Presione Enter para volver al menu principal");
}

static void mostrarMenu()
{
    std::cout << "\n";
    std::cout << "============================================================\n";
    std::cout << "     MINI-KERNEL DE DEMOSTRACION - SISTEMAS OPERATIVOS     \n";
    std::cout << "   Universidad Distrital Francisco Jose de Caldas - 2026   \n";
    std::cout << "============================================================\n\n";
    std::cout << "Seleccione una opcion:\n\n";
    std::cout << "  1. Fase 1 - Gestion de Procesos        \n";
    std::cout << "  2. Fase 2 - Gestion de Memoria         \n";
    std::cout << "  3. Fase 3 - Sistema de Archivos y E/S  \n";
    std::cout << "  4. Demostracion Completa (Kernel)      \n";
    std::cout << "  5. Salir\n\n";
    std::cout << "Opcion: ";
}

int main()
{
    int opcion = 0;

    while (true)
    {
        mostrarMenu();
        std::cin >> opcion;

        if (std::cin.fail())
        {
            limpiarBuffer();
            std::cout << "\n[ERROR] Opcion invalida. Intente de nuevo.\n";
            continue;
        }

        limpiarBuffer();

        switch (opcion)
        {
        case 1:
            demoFase1();
            break;
        case 2:
            demoFase2();
            break;
        case 3:
            demoFase3();
            break;
        case 4:
            demoCompleta();
            break;
        case 5:
            std::cout << "\nGracias por usar el Mini-Kernel!\n\n";
            return 0;
        default:
            std::cout << "\n[ERROR] Opcion invalida. Seleccione 1-5.\n";
        }
    }

    return 0;
}