#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p resultados
{
  date -Iseconds
  uname -a
  lscpu | grep -E "Model name|^CPU\(s\)|Thread|Core"
  gcc --version | head -1
  python3 -c "import sympy, polars, matplotlib; print('sympy', sympy.__version__, 'polars', polars.__version__, 'matplotlib', matplotlib.__version__)"
  if command -v nvidia-smi > /dev/null; then nvidia-smi --query-gpu=name,compute_cap,driver_version --format=csv,noheader; fi
  if command -v nvcc > /dev/null; then nvcc --version | tail -1; fi
} > resultados/entorno.txt
make all
make test
for caso in "50 200 7" "300 2000 11"; do
  python3 oraculo.py $caso
  gcc -O2 -o compara compara.c poli.c -lm
  ./compara
done
./bench.sh 1000 100000 5 resultados/tiempos.csv
python3 graficas.py resultados/tiempos.csv
if command -v nvidia-smi > /dev/null; then
  make grad
  for b in 32 64 128 256 512 1024; do ./grad 10000 1000000 "$b"; done > resultados/cuda.txt
fi
echo "Resultados en resultados/"
