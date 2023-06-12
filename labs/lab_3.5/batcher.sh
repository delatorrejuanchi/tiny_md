#!/bin/bash

for script_file in labs/lab_3.5/scripts/*.sh; do
    # Extract the filename without the extension to use as job name
    script_name=$(basename "$script_file" .sh)

    # Create a SLURM script
    echo "#!/bin/bash
#SBATCH --job-name=tiny_md-${script_name}
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32
#SBATCH --exclusive
srun sh $script_file" > "${script_name}.slurm"

    # Submit the SLURM script
    sbatch "${script_name}.slurm"
done