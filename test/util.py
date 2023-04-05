def average_tiny_md(num_runs: int) -> None:
    outputs = [parse_output(run_tiny_md()) for _ in range(num_runs)]

    print("average output:")
    avg = np.mean(outputs, axis=0)
    print(" ".join([f"{x:.6f}" for x in row for row in avg]))

    pairwise_diffs = np.array(
        [np.abs(outputs[i] - outputs[j])
         for i in range(num_runs) for j in range(i + 1, num_runs)]
    )

    mean_diffs = np.mean(pairwise_diffs, axis=0)
    print("mean of differences:")
    print("mean: ", np.mean(mean_diffs, axis=0).astype(float))
    print("std: ", np.std(mean_diffs, axis=0).astype(float))

    std_diffs = np.std(pairwise_diffs, axis=0)
    print("std of differences:")
    print("mean: ", np.mean(std_diffs, axis=0).astype(float))
    print("std: ", np.std(std_diffs, axis=0).astype(float))
