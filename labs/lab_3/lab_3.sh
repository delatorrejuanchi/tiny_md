rm .depend
make clean

# lab 2 baseline
git checkout main
python3 benchmark/benchmark_atom.py baseline --n-values 256 500 864 1372 2048 --num-runs 2

# lab 3 - vectorization improvements
git checkout lab3-vectorization
python3 benchmark/benchmark_atom.py vectorization --n-values 256 500 864 1372 2048 --num-runs 2

# lab 3 - openmp
git checkout lab3-openmp

# 1 thread
python3 benchmark/benchmark_atom.py openmp_1_threads --n-values 256 500 864 1372 2048 2916 4000 5324 6912 --n-threads 1 --num-runs 2

# 2 threads
python3 benchmark/benchmark_atom.py openmp_2_threads --n-values 256 500 864 1372 2048 2916 4000 5324 6912 --n-threads 2 --num-runs 2

# 4 threads
python3 benchmark/benchmark_atom.py openmp_4_threads --n-values 256 500 864 1372 2048 2916 4000 5324 6912 --n-threads 4 --num-runs 2

# 8 threads
python3 benchmark/benchmark_atom.py openmp_8_threads --n-values 256 500 864 1372 2048 2916 4000 5324 6912 --n-threads 8 --num-runs 2

# 16 threads
python3 benchmark/benchmark_atom.py openmp_16_threads --n-values 256 500 864 1372 2048 2916 4000 5324 6912 --n-threads 16 --num-runs 2

# 32 threads
python3 benchmark/benchmark_atom.py openmp_32_threads --n-values 256 500 864 1372 2048 2916 4000 5324 6912 --n-threads 32 --num-runs 2
