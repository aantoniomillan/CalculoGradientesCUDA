#!/usr/bin/env bash
set -euo pipefail
nVars=${1:-1000}
nMon=${2:-100000}
reps=${3:-5}
salida=${4:-tiempos.csv}
echo "hilos,rep,variante,tiempo" > "$salida"
for hilos in 1 2; do
  for rep in $(seq 1 "$reps"); do
    OMP_NUM_THREADS=$hilos OMP_PROC_BIND=close OMP_PLACES=cores ./omp "$nVars" "$nMon" 16 |
      awk -v h="$hilos" -v r="$rep" 'NR > 1 { print h "," r "," $1 "," $2 }' >> "$salida"
  done
done
echo "Escrito $salida"
