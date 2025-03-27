#!/bin/bash

# Source Intel oneAPI environment
. /opt/intel/oneapi/setvars.sh --include-intel-llvm > /dev/null 2>&1

# Activate virtual environment
source .venv/bin/activate

# Define directories
dataSet_dir="testSet"
model_dir="$dataSet_dir/Model"
rawData_dir="$dataSet_dir/rawData"
histogram_dir="$dataSet_dir/histogram"
depthImage_dir="$dataSet_dir/depthImage"
globalPrefix="test"

# Create directories if they don't exist
mkdir -p "$dataSet_dir"
mkdir -p "$model_dir"
mkdir -p "$rawData_dir"
mkdir -p "$histogram_dir"

for ((i=1; i<=1; i++))
do
    model_file="${globalPrefix}_obj_${i}.obj"
    model_file_path="${model_dir}/${model_file}"
    python3 scenGen.py "${model_file_path}"|| { echo "Error running scenGen.py"; exit 1; }

    rawData_file="${globalPrefix}_rawData_${i}.h5"
    rawData_file_path="${rawData_dir}/${rawData_file}"
    
    ./syclImplementation/build/LiDARSimulation \
    --model "${model_file_path}" \
    --output "${rawData_file_path}" || { 
    echo "Error running HELLOEMBREE"; 
    exit 1;} 

    histogram_file="${globalPrefix}_histogram_${i}.h5"
    histogram_file_path="${histogram_dir}/${histogram_file}"

    python3 lens.py --input_file "${rawData_file_path}" --output_file "${histogram_file_path}"|| { echo "Error running lens.py"; exit 1; }

    depthImage_file="${globalPrefix}_depth${i}.png"
    depthImage_file_path="${depthImage_dir}/${depthImage_file}"

    python3 fromImage.py "${histogram_file_path}" "${depthImage_file_path}"  || { echo "Error running fromImage.py"; exit 1;}

    echo "Iteration ${i} completed"

done
