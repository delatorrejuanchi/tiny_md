import subprocess

flags_map = {
    "gcc": [
        "-O0",
        "-O1",
        "-O2",
        "-O3",
        "-Ofast",
    ],
    "clang": [
        "-O0",
        "-O1",
        "-O2",
        "-O3",
        "-Ofast",
    ],
}


def build(compiler, flags):
    command = f"make clean && make CC={compiler} CFLAGS='{flags}'"
    subprocess.run(command, shell=True, check=True)


def run_benchmark(name):
    command = f"make name={name} benchmark"
    subprocess.run(command, shell=True, check=True)


def run_plot():
    command = "make name=compilers plot"
    subprocess.run(command, shell=True, check=True)


if __name__ == "__main__":
    for compiler, flags_list in flags_map.items():
        for flags in flags_list:
            build(compiler, flags)

            benchmark = f"{compiler}_{flags.replace('-', '').replace(' ', '_')}"
            run_benchmark(benchmark)

    run_plot()
