#!/bin/bash

#SBATCH --job-name=tiny_md-delatorre
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32
#SBATCH --exclusive
#SBATCH --time=03:00:00

srun sh labs/lab_3/lab_3.sh