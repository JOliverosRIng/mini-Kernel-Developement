CXX      := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -O2
INCLUDE  := -Iinclude

MAIN_SRC := src/main.cpp src/memory.cpp src/paging.cpp src/process.cpp
TEST_SRC := tests/test_memory.cpp src/memory.cpp src/paging.cpp src/process.cpp

.PHONY: all run test clean

all: mini_kernel

mini_kernel: $(MAIN_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $@
	@echo "Compilacion exitosa: ./mini_kernel"

run: mini_kernel
	./mini_kernel

test: test_memory
	./test_memory

test_memory: $(TEST_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $@

clean:
	rm -rf mini_kernel test_memory *.dSYM
