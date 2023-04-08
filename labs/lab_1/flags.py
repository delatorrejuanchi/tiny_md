import subprocess

flags_list = [
    "-O0",
    "-O1",
    "-O2",
    "-O3",
    "-Ofast",
    "-O3 -march=native",
    "-Ofast -march=native",
    "-Ofast -march=native -funsafe-math-optimizations",
    "-Ofast -march=native -ffinite-math-only",
    "-Ofast -march=native -funsafe-math-optimizations -ffinite-math-only",
    "-Ofast -march=native -funsafe-math-optimizations -ffinite-math-only -funroll-loops",
]


def build(flags):
    command = f"make clean && make CC=gcc CFLAGS='{flags}'"
    subprocess.run(command, shell=True, check=True)


def run_benchmark(name):
    command = f"make name={name} benchmark"
    subprocess.run(command, shell=True, check=True)


def run_plot():
    command = "make name=flags plot"
    subprocess.run(command, shell=True, check=True)


if __name__ == "__main__":
    for flags in flags_list:
        build(flags)
        run_benchmark(f"gcc_{flags.replace('-', '').replace(' ', '_')}")

    run_plot()
