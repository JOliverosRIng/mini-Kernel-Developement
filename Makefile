CXX      := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -O2
INCLUDE  := -Isrc

APP_SRC := src/main.cpp src/Kernel.cpp src/memory.cpp src/paging.cpp src/process.cpp src/filesystem.cpp src/io.cpp src/Scheduler.cpp
APP_LDFLAGS := -mwindows -luser32

TEST_SRC := tests/test_memory.cpp src/memory.cpp src/paging.cpp src/process.cpp
TEST_FS_SRC := tests/test_filesystem.cpp src/filesystem.cpp src/io.cpp

.PHONY: all run test clean

all: mini_kernel

mini_kernel: $(APP_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $@ $(APP_LDFLAGS)
	@echo "Compilacion exitosa: ./mini_kernel.exe"

run: mini_kernel
	.\mini_kernel.exe

test: test_memory test_filesystem
	@echo.
	@echo === Ejecutando Tests de Fase 2 (Memoria) ===
	.\test_memory.exe
	@echo.
	@echo === Ejecutando Tests de Fase 3 (FileSystem + IO) ===
	.\test_filesystem.exe
	@echo.
	@echo === Todos los tests completados ===

test_memory: $(TEST_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $@

test_filesystem: $(TEST_FS_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $@

clean:
	cmd /c if exist mini_kernel.exe del /F /Q mini_kernel.exe & if exist test_memory.exe del /F /Q test_memory.exe & if exist test_filesystem.exe del /F /Q test_filesystem.exe
