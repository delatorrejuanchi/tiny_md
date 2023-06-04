import argparse
import os
import subprocess
import re
from pathlib import Path
from typing import List
import numpy as np
import csv

DEFAULT_N_VALUES = [256]
DEFAULT_NTHREADS = 8
DEFAULT_NUM_RUNS = 20


def run_tiny_md(N: int, NTHREADS: int) -> str:
    subprocess.run(
        ["rm", ".depend"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=False
    )

    subprocess.run(
        ["make", "clean"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=True
    )

    result = subprocess.run(
        ["make", f'N=-DN={N}', f'N_THREADS=-DN_THREADS={NTHREADS}'],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=True
    )

    result = subprocess.run(
        ["./tiny_md"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=True
    )

    return result.stdout


def benchmark(name: str, n_values: List[int], n_threads: int, num_runs: int):
    avg_performances = []
    std_devs = []

    print(f"Running benchmark with {n_values} particles {num_runs} times")
    for n in n_values:
        run_metrics = []
        for i in range(num_runs):
            print(f"N = {n}: {i+1}/{num_runs}...", end=" ", flush=True)
            metric = get_performance_from_output(run_tiny_md(n, n_threads))
            run_metrics.append(metric)
            print("OK")

        avg_performance = np.mean(run_metrics)
        std_dev = np.std(run_metrics)

        avg_performances.append(avg_performance)
        std_devs.append(std_dev)

    save_to_file(name, n_values, avg_performances, std_devs)


def get_performance_from_output(output: str) -> float:
    num_particles_pattern = r"# Número de partículas:\s+(\d+)"
    total_time_pattern = r"# Tiempo total de simulación = (\d+\.\d+) segundos"

    num_particles_match = re.search(num_particles_pattern, output)
    total_time_match = re.search(total_time_pattern, output)

    if not num_particles_match or not total_time_match:
        raise ValueError("Missing information in the output")

    num_particles = int(num_particles_match.group(1))
    total_time = float(total_time_match.group(1))

    # Metric proportional to interactions per second
    return (num_particles * (num_particles - 1)) / total_time


def save_to_file(name: str, n_values: List[int], avg_times: List[float], std_devs: List[float]):
    stats_file = os.path.join("benchmark/stats", f"{name}.csv")

    data = []
    if os.path.exists(stats_file):
        with open(stats_file, 'r') as f:
            reader = csv.DictReader(f)
            for row in reader:
                data.append(row)

    for n, avg, std in zip(n_values, avg_times, std_devs):
        for item in data:
            if int(item["N"]) == n:
                item["avg"] = avg
                item["std"] = std
                break
        else:
            data.append({"N": n, "avg": avg, "std": std})

    with open(stats_file, 'w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=["N", "avg", "std"])
        writer.writeheader()
        for row in data:
            writer.writerow(row)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "name",
        help="Name for the benchmark. Used as the filename of the output csv file or plot."
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
    parser.add_argument(
        "--n-threads",
        type=int,
        metavar="NTHREADS",
        default=DEFAULT_NTHREADS,
        help=f"Number of threads to use. Default is {DEFAULT_NTHREADS}."
    )
    args = parser.parse_args()

    benchmark(args.name, args.n_values, args.n_threads, args.num_runs)
