import os
import subprocess
import pandas as pd
import numpy as np
from io import StringIO
from typing import List
import sys
import argparse

COLUMN_NAMES = [
    "densidad",
    "volumen",
    "energia_potencial_media",
    "presion_media"
]

DEFAULT_N = 256
DEFAULT_NUM_RUNS = 20
DEFAULT_SUCCESS_THRESHOLD = 0.9


def run_tiny_md(N: int) -> str:
    subprocess.run(
        ["rm", ".depend"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=True
    )

    subprocess.run(
        ["make", "clean"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=True
    )

    subprocess.run(
        ["make", f'USER_DEFINES="-DN={N}"'],
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


def parse_output(output: str) -> pd.DataFrame:
    lines = [line for line in output.split('\n') if not line.startswith('#')]
    cleaned_output = '\n'.join(lines)

    data = pd.read_csv(
        StringIO(cleaned_output),
        delim_whitespace=True,
        header=None
    )

    return data


def test_output(expected_output: pd.DataFrame, actual_output: pd.DataFrame, mean_epsilon_values: List[float], std_epsilon_values: List[float]) -> None:
    diff = np.abs(expected_output - actual_output)
    mean_diff = diff.mean(axis=0)
    std_diff = diff.std(axis=0)

    for col, (epsilon, std_epsilon) in enumerate(zip(mean_epsilon_values, std_epsilon_values)):
        assert mean_diff[col] <= epsilon, f"Mean difference of column {COLUMN_NAMES[col]} is too large: {mean_diff[col]}"
        assert std_diff[col] <= std_epsilon, f"Standard deviation of column {COLUMN_NAMES[col]} is too large: {std_diff[col]}"


def test(N: int, num_runs: int, success_threshold: float) -> None:
    print(f"Running test for tiny_md with {N} particles {num_runs} times...")

    with open(os.path.join("test", f"expected_output_{N}.txt"), "r") as f:
        expected_output = parse_output(f.read())

    with open(os.path.join("test", f"expected_stats_{N}.txt"), "r") as f:
        mean_epsilon_values, std_epsilon_values = (
            [float(x) for x in line.split()] for line in f.read().split("\n"))

    successful_runs = 0
    for i in range(num_runs):
        print(f"Run {i + 1}/{num_runs}...", end=" ")
        output = parse_output(run_tiny_md(N))

        try:
            test_output(
                expected_output,
                output,
                mean_epsilon_values,
                std_epsilon_values
            )

            successful_runs += 1
            print("OK")
        except AssertionError as e:
            print("ERROR: ", e)
            print("Expected output values:")
            print(expected_output)
            print("Actual output values:")
            print(output)

    success_rate = successful_runs / num_runs
    print(f"Success rate: {success_rate * 100:.1f}%")
    print(f"Success threshold: {success_threshold * 100:.1f}%")

    if success_rate < success_threshold:
        print("FAIL")
        sys.exit(1)

    print("OK")


def compute_statistics(N: int, num_runs: int) -> None:
    print(
        f"Computing statistics for tiny_md with {N} particles after {num_runs} runs...")

    outputs = []
    for i in range(num_runs):
        print(f"Run {i + 1}/{num_runs}...", end=" ")

        outputs.append(parse_output(run_tiny_md(N)))

        print("OK")

    avg = np.mean(outputs, axis=0)

    with open(os.path.join("test", f"expected_output_{N}.txt"), "w") as f:
        print("Writing expected output to file...", end=" ")

        for row in avg:
            f.write(" ".join([f"{x:.6f}" for x in row]))
            f.write("\n")

        print("OK")

    diffs = [np.abs(output - avg) for output in outputs]
    mean_diffs = np.mean(diffs, axis=0)
    std_diffs = np.std(diffs, axis=0)
    with open(os.path.join("test", f"expected_stats_{N}.txt"), "w") as f:
        print("Writing expected statistics to file...", end=" ")

        # Set the epsilon values to 2 standard deviations from the mean, which should cover ~95% of the cases.
        mean_epsilon_values = np.mean(
            mean_diffs, axis=0) + 2 * np.std(mean_diffs, axis=0)
        std_epsilon_values = np.mean(
            std_diffs, axis=0) + 2 * np.std(std_diffs, axis=0)

        f.write(" ".join([f"{x:.6f}" for x in mean_epsilon_values]))
        f.write("\n")
        f.write(" ".join([f"{x:.6f}" for x in std_epsilon_values]))

        print("OK")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Test script for tiny_md")

    parser.add_argument(
        "--mode",
        choices=["test", "compute-statistics"],
        default="test",
        help="Mode of operation. Can be 'test' or 'compute-statistics'. Default is 'test'.",
    )
    parser.add_argument(
        "--n",
        type=int,
        metavar="N",
        default=DEFAULT_N,
        help=f"Number of particles. Default is {DEFAULT_N}.",
    )
    parser.add_argument(
        "--num-runs",
        type=int,
        metavar="NUM_RUNS",
        default=DEFAULT_NUM_RUNS,
        help=f"Number of times to run tiny_md. Default is {DEFAULT_NUM_RUNS}.",
    )
    parser.add_argument(
        "--success-threshold",
        type=float,
        metavar="SUCCESS_THRESHOLD",
        default=DEFAULT_SUCCESS_THRESHOLD,
        help=f"Success rate threshold for the test mode. Default is {DEFAULT_SUCCESS_THRESHOLD}.",
    )

    args = parser.parse_args()

    if args.mode == "compute-statistics":
        compute_statistics(args.n, args.num_runs)
    else:
        test(args.n, args.num_runs, args.success_threshold)
