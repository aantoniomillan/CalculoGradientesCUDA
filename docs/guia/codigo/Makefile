CC = gcc
NVCC = nvcc
CFLAGS = -O3 -Wall -Wextra -std=c11
ARCH = -arch=sm_89

all: ejemplo jac omp

ejemplo: ejemplo.c poli.c poli.h
	$(CC) $(CFLAGS) -o $@ ejemplo.c poli.c -lm

jac: jac.c poli.c poli.h
	$(CC) $(CFLAGS) -o $@ jac.c poli.c -lm

omp: omp.c poli.c poli.h
	$(CC) $(CFLAGS) -fopenmp -o $@ omp.c poli.c -lm

grad: grad.cu
	$(NVCC) -O3 $(ARCH) -Xptxas=-v -o $@ grad.cu

test: ejemplo jac
	./ejemplo | grep -q "grad = (8, 15, 8)" && echo "ejemplo OK"
	./jac | grep -q "fila = (9, 24, 25)" && echo "jacobiano OK"

clean:
	rm -f ejemplo jac omp grad compara gradRs
