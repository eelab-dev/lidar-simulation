#!/bin/bash

# Set execution directory (the directory where this script is located)
SCRIPT_DIR=$(dirname "$(realpath "$0")")
cd "$SCRIPT_DIR" || exit 1

# Source Intel oneAPI environment
. /opt/intel/oneapi/setvars.sh --include-intel-llvm > /dev/null 2>&1

# Activate virtual environment
source .venv/bin/activate

# Define directories
dataSet_dir="negative"
model_dir="$dataSet_dir/Model"
rawData_dir="$dataSet_dir/rawData"
histogram_dir="$dataSet_dir/histogramData"
pixelized_dir="$dataSet_dir/pixelizedData"
depthImage_dir="$dataSet_dir/depthImage"
globalPrefix="negative"

positive_globalPrefix="positive"
positive_dir="positive"
# Create directories if they don't exist
mkdir -p "$dataSet_dir"
mkdir -p "$model_dir"
mkdir -p "$rawData_dir"
mkdir -p "$histogram_dir"

for ((i=1; i<=1; i++))
do
    positive_obj_file="${positive_dir}/model/${positive_globalPrefix}_obj_${i}.obj"
    model_file="${globalPrefix}_obj_${i}.obj"
    model_file_path="${model_dir}/${model_file}"
    python3 pythonScripts/removeSmallBox.py \
    --input_file "${positive_obj_file}"\
    --output_file "${model_file_path}" \
     
    rawData_file="${globalPrefix}_rawData_${i}.h5"
    rawData_file_path="${rawData_dir}/${rawData_file}"
    
    ./syclImplementation/build/LiDARSimulation \
    --model "${model_file_path}" \
    --output "${rawData_file_path}" \
    --seed 321 || { 
    echo "Error running HELLOEMBREE"; 
    exit 1;} 

    pixelized_file="${globalPrefix}_pixelized_${i}.h5"
    pixelized_file_path="${pixelized_dir}/${pixelized_file}${i}.h5"

    python3 pythonScripts/pixelation.py \
    --input_file "${rawData_file_path}" \
    --output_file "${pixelized_file_path}"|| {
    echo "Error running lens.py"; 
    exit 1; }

    histogram_file="${globalPrefix}_histogram_${i}.h5"
    histogram_file_path="${histogram_dir}/${histogram_file}"

    depthImage_file="${globalPrefix}_depth${i}.png"
    depthImage_file_path="${depthImage_dir}/${depthImage_file}"

    python3 pythonScripts/fromImage.py \
    --input_file "${pixelized_file_path}"\
    --output_image "${depthImage_file_path}"\
    --output_file "${histogram_file_path}" || {
    echo "Error running fromImage.py"; 
    exit 1;}

    echo "Iteration ${i} completed"

done