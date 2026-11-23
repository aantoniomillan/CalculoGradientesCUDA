CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -fopenmp -Isrc
NVCC := $(shell command -v nvcc 2>/dev/null)

SRC := src/main.cpp src/datos.cpp $(wildcard src/variantes/*.cpp)
OBJ := $(SRC:src/%.cpp=build/%.o)
LDLIBS := -fopenmp

ifeq ($(NVCC),)
CXXFLAGS += -DSIN_CUDA
else
CUDA_DIR := $(patsubst %/bin/nvcc,%,$(NVCC))
OBJ += build/variantes/cuda.o
LDLIBS += -L$(CUDA_DIR)/lib64 -lcudart
endif

gradientes: $(OBJ)
	$(CXX) $^ -o $@ $(LDLIBS)

build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/%.o: src/%.cu
	@mkdir -p $(dir $@)
	$(NVCC) -O2 -Isrc -c $< -o $@

clean:
	rm -rf build gradientes

.PHONY: clean
