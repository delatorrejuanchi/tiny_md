python3 -m venv pyenv

# Activate the virtual environment
source pyenv/bin/activate

# Install the required packages
pip3 install pandas numpy matplotlib

rebuild() {
    rm .depend

    # Clean the directory
    make clean

    # Compile the code
    make
}

# Run the code
rebuild && python3 labs/lab_1/compilers.py
mv benchmark/stats/* labs/lab_1/compilers

rebuild && python3 labs/lab_1/flags.py
mv benchmark/stats/* labs/lab_1/flags

# optimizations
# baseline
rebuild && make name=baseline benchmark

# branch_prediction_misses
git checkout optimization-branch_prediction_misses
rebuild && make name=branch_prediction_misses benchmark

# float
git checkout optimization-float
rebuild && make name=float benchmark

# float-branch_prediction_misses
git checkout optimization-float-branch_prediction_misses
rebuild && make name=float-branch_prediction_misses benchmark

mv benchmark/stats/* labs/lab_1/optimization
