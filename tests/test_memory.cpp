
#include "memory.hpp"
#include "paging.hpp"
#include <iostream>
#include <cassert>

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

static void testMemoryAllocate()
{
    std::cout << "\n[TEST] MemoryManager::allocate\n";
    MemoryManager mem;

    int a1 = mem.allocate(1, 100);
    TEST("First-Fit: primer bloque en dirección 0", a1 == 0);

    int a2 = mem.allocate(2, 200);
    TEST("Second alloc starts at 100", a2 == 100);

    int a3 = mem.allocate(3, 50);
    TEST("Third alloc starts at 300", a3 == 300);

    int a4 = mem.allocate(4, MEM_SIZE);
    TEST("Alloc > available returns -1", a4 == -1);

    mem.printMap();
}

static void testMemoryFreeAndCoalesce()
{
    std::cout << "\n[TEST] MemoryManager::free + coalesce\n";
    MemoryManager mem;

    mem.allocate(1, 200);
    mem.allocate(2, 200);
    mem.allocate(3, 200);

    mem.free(2);

    int a = mem.allocate(4, 150);
    TEST("Alloc in freed block (First-Fit)", a == 200);

    mem.free(1);
    mem.free(4);

    int bigAlloc = mem.allocate(5, 380);
    TEST("Coalescing allows larger alloc", bigAlloc == 0);

    mem.printMap();
}

static void testMemoryInvalidSize()
{
    std::cout << "\n[TEST] MemoryManager — tamaños inválidos\n";
    MemoryManager mem;

    int r1 = mem.allocate(1, 0);
    TEST("Alloc size=0 returns -1", r1 == -1);

    int r2 = mem.allocate(1, -10);
    TEST("Alloc size negative returns -1", r2 == -1);

    int r3 = mem.allocate(1, MEM_SIZE + 1);
    TEST("Alloc size > MEM_SIZE returns -1", r3 == -1);
}

static void testPagingTranslate()
{
    std::cout << "\n[TEST] PagingUnit::translate\n";
    MemoryManager mem;
    PagingUnit paging;

    int base = mem.allocate(1, 128);
    paging.createTable(1, base, 128);

    int p1 = paging.translate(1, 0);
    TEST("Lógica 0 → física 0", p1 == 0);

    int p2 = paging.translate(1, 63);
    TEST("Lógica 63 → física 63 (último de página 0)", p2 == 63);

    int p3 = paging.translate(1, 64);
    TEST("Lógica 64 → física 64 (inicio de página 1)", p3 == 64);

    int p4 = paging.translate(1, 127);
    TEST("Lógica 127 → física 127 (último byte)", p4 == 127);

    int p5 = paging.translate(1, 200);
    TEST("Lógica 200 fuera de rango → fallo (-1)", p5 == -1);

    int p6 = paging.translate(99, 0);
    TEST("PID sin tabla → -1", p6 == -1);

    paging.printTable(1);
}

static void testPagingRemove()
{
    std::cout << "\n[TEST] PagingUnit::removeTable\n";
    MemoryManager mem;
    PagingUnit paging;

    int base = mem.allocate(1, 64);
    paging.createTable(1, base, 64);

    paging.removeTable(1);
    int r = paging.translate(1, 0);
    TEST("Tras removeTable translate retorna -1", r == -1);
}

int main()
{
    std::cout << "╔═══════════════════════════════════════════╗\n";
    std::cout << "║   TEST SUITE — Fase 2: Gestión de Memoria ║\n";
    std::cout << "╚═══════════════════════════════════════════╝\n";

    testMemoryAllocate();
    testMemoryFreeAndCoalesce();
    testMemoryInvalidSize();
    testPagingTranslate();
    testPagingRemove();

    std::cout << "\n─────────────────────────────────────────────\n";
    std::cout << "Resultados: " << passed << " pasaron, "
              << failed << " fallaron.\n";

    return (failed == 0) ? 0 : 1;
}
