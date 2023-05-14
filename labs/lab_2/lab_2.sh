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

# lab 2 vectorization
git checkout lab2-vectorization
make name=vectorization benchmark

make name=lab2 plot
