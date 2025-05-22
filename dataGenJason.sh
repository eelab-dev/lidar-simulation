#!/bin/bash

# Set execution directory (the directory where this script is located)
SCRIPT_DIR=$(dirname "$(realpath "$0")")
cd "$SCRIPT_DIR" || exit 1

# Source Intel oneAPI environment
. /opt/intel/oneapi/setvars.sh --include-intel-llvm > /dev/null 2>&1

# Activate virtual environment
source .venv/bin/activate


config="STM32Lidar/stm32config.json"
config_dir=$(dirname "$config")

iteration=$(jq -r '.global_settings.iteration // empty' "$config")
global_prefix=$(jq -r '.global_settings.global_prefix // empty' "$config") 

if [ -z "$iteration" ]; then
    echo "'iteration' not found."
    exit 1;
fi

if [ -z "$global_prefix" ]; then
    echo "'global_prefix' not found."
    exit 1;
fi

for ((i=1;i<=iteration;i++))
do

    if jq -e '.model_generation | length > 0' "$config" > /dev/null 2>&1; then
        model_dir=$(jq -r '.model_generation.output_model_dir' "$config")
        model_dir="${config_dir}/${model_dir}"
        mkdir -p "$model_dir"
        model_file="${global_prefix}_obj_${i}.obj"
        model_file_path="${model_dir}/${model_file}"
        python3 pythonScripts/scenGen.py \
        --output_file "${model_file_path}"|| { 
        echo "Error running scenGen.py";
        exit 1; }
    fi
    

    if jq -e '.simulation_generation | length > 0' "$config" > /dev/null 2>&1; then
        input_model_dir=$(jq -r '.simulation_generation.input_model_dir' "$config")
        input_model_dir="${config_dir}/${input_model_dir}"
        input_model_file="${global_prefix}_obj_${i}.obj"
        input_model_file_path="${input_model_dir}/${input_model_file}"


        rawData_dir=$(jq -r '.simulation_generation.output_rawData_dir' "$config")
        rawData_dir="${config_dir}/${rawData_dir}"
        mkdir -p "$rawData_dir"

        if [  -f "$input_model_file_path" ]; then
            inputFov=$(jq -r '.simulation_generation.fov // 38' "$config")  # default to 38 if missing
            rawData_file="${globalPrefix}_rawData_${i}.h5"
            rawData_file_path="${rawData_dir}/${rawData_file}"

            ./syclImplementation/build/LiDARSimulation \
            --model "${input_model_file_path}" \
            --output "${rawData_file_path}" \
            --fov "${inputFov}"\
            --seed $i|| { 
            echo "Error running HELLOEMBREE"; 
            exit 1;} 
        else 
            echo "Skipping simulation block: $input_model_file_path does not exist"
        fi
    fi


    
done
