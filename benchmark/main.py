import argparse
import os
import subprocess
import re
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path
from typing import List
import pandas as pd

DEFAULT_N_VALUES = [256]
DEFAULT_NUM_RUNS = 20


def run_tiny_md(N: int) -> str:
    result = subprocess.run(
        ["./tiny_md", str(N)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=True
    )

    return result.stdout


def benchmark(name: str, n_values: List[int], num_runs: int):
    avg_times = []
    std_devs = []

    print(f"Running benchmark with {n_values} particles {num_runs} times")
    for n in n_values:
        print(f"Warming up for N = {n}...", end=" ", flush=True)
        run_tiny_md(n)  # Warmup
        print("OK")

        run_times = []
        for i in range(num_runs):
            print(f"N = {n}: {i}/{num_runs}...", end=" ", flush=True)
            run_times.append(get_performance_from_output(run_tiny_md(n)))
            print("OK")

        avg_time = np.mean(run_times)
        std_dev = np.std(run_times)

        avg_times.append(avg_time)
        std_devs.append(std_dev)

    save_to_file(name, n_values, avg_times, std_devs)


def get_performance_from_output(output: str) -> float:
    num_particles_pattern = r"# Número de partículas:\s+(\d+)"
    total_time_pattern = r"# Tiempo total de simulación = (\d+\.\d+) segundos"
    simulated_time_pattern = r"# Tiempo simulado = (\d+\.\d+) \[fs\]"

    num_particles_match = re.search(num_particles_pattern, output)
    total_time_match = re.search(total_time_pattern, output)
    simulated_time_match = re.search(simulated_time_pattern, output)

    if not num_particles_match or not total_time_match or not simulated_time_match:
        raise ValueError("Missing information in the output")

    num_particles = int(num_particles_match.group(1))
    total_time = float(total_time_match.group(1))
    simulated_time = float(simulated_time_match.group(1))

    # return (num_particles * (num_particles - 1) / 2) * simulated_time / total_time
    return total_time


def save_to_file(name: str, n_values: List[int], avg_times: List[float], std_devs: List[float]):
    stats_file = os.path.join("benchmark/stats", f"{name}.csv")

    if os.path.exists(stats_file):
        df = pd.read_csv(stats_file)
    else:
        df = pd.DataFrame(columns=["N", "avg", "std"])

    for n, avg, std in zip(n_values, avg_times, std_devs):
        if (df.N == n).any():
            df.loc[df.N == n, ["avg", "std"]] = avg, std
        else:
            new_row = pd.DataFrame({"N": [n], "avg": [avg], "std": [std]})
            df = pd.concat([df, new_row], ignore_index=True)

    df.to_csv(stats_file, index=False)


def plot_benchmark_stats(name: str):
    stats_files = sorted(
        Path("benchmark/stats").glob("*.csv"), key=os.path.getctime)

    for filepath in stats_files:
        print(f"Reading {filepath}...")
        df = pd.read_csv(filepath)
        df.sort_values(by=["N"], inplace=True)

        n_values = df["N"].values
        avg_times = df["avg"].values
        std_devs = df["std"].values

        plt.errorbar(n_values, avg_times, yerr=std_devs,
                     capsize=5, label=filepath.name)

    plt.xlabel("N")
    plt.ylabel("Average Time (s)")
    plt.legend()
    plt.title("Benchmark Stats")

    plt.savefig(f"benchmark/plots/{name}.png", dpi=300)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "name",
        help="Name for the benchmark. Used as the filename of the output csv file or plot."
    )
    parser.add_argument(
        "--mode",
        choices=["benchmark", "plot"],
        default="benchmark",
        help="Mode of operation. Can be 'benchmark' or 'plot'. Default is 'benchmark'.",
    )
    parser.add_argument(
        "--num-runs",
        type=int,
        metavar="NUM_RUNS",
        default=DEFAULT_NUM_RUNS,
        help=f"Number of times to run the simulation. Default is {DEFAULT_NUM_RUNS}."
    )
    parser.add_argument(
        "--n-values",
        nargs='+',
        type=int,
        metavar="N",
        default=DEFAULT_N_VALUES,
        help=f"List of N values to use. Default is {DEFAULT_N_VALUES}."
    )
    args = parser.parse_args()

    if args.mode == "benchmark":
        benchmark(args.name, args.n_values, args.num_runs)

    elif args.mode == "plot":
        plot_benchmark_stats(args.name)
