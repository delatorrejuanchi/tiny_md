# python3 -m venv pyenv

# # Activate the virtual environment
# source pyenv/bin/activate

# # Install the required packages
# pip3 install pandas numpy matplotlib

# clean

rm .depend
make clean

# lab 1 baseline
git checkout main
make
make name=baseline benchmark

# lab 2 vectorization (AoS)
git checkout lab2-vectorization
make name=vectorization_aos benchmark

# lab 2 vectorization - float (AoS)
git checkout lab2-vectorization-float
make name=vectorization_aos_float benchmark

# lab 2 vectorization (SoA)
git checkout lab2-vectorization-soa
make name=vectorization_soa benchmark

# lab 2 vectorization - float (SoA)
git checkout lab2-vectorization-soa-float
make name=vectorization_soa_float benchmark

make name=lab2 plot
