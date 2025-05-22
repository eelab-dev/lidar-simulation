#!/bin/bash

# Set execution directory (the directory where this script is located)
SCRIPT_DIR=$(dirname "$(realpath "$0")")
cd "$SCRIPT_DIR" || exit 1

# Source Intel oneAPI environment
. /opt/intel/oneapi/setvars.sh --include-intel-llvm > /dev/null 2>&1

# Activate virtual environment
source .venv/bin/activate


config="STM32Lidar/negative/stm32_negative_config.json"
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


    if jq -e '.objectRemove_process | length > 0' "$config" > /dev/null 2>&1; then
        input_model_dir=$(jq -r '.objectRemove_process.input_model_dir' "$config")
        input_model_dir="${config_dir}/${input_model_dir}"
        input_model_file_prefix=$(jq -r '.objectRemove_process.input_model_file_prefix' "$config")
        input_model_file="${input_model_file_prefix}_obj_${i}.obj"
        input_model_file_path="${input_model_dir}/${input_model_file}"

        if [ -f "$input_model_file_path" ]; then
            output_model_dir=$(jq -r '.objectRemove_process.output_model_dir' "$config")
            output_model_dir="${config_dir}/${output_model_dir}"
            mkdir -p "$output_model_dir"

            output_model_file="${global_prefix}_obj_${i}.obj"
            output_model_file_path="${output_model_dir}/${output_model_file}"

            python3 pythonScripts/removeSmallBox.py \
                --input_file "${input_model_file_path}" \
                --output_file "${output_model_file_path}" || {
                    echo "Error running removeSmallBox.py on ${input_model_file_path}"
                    exit 1
                }
        else
            echo "Skipping object removal: $input_model_file_path does not exist"
        fi
    fi
    

    if jq -e '.simulation_generation | length > 0' "$config" > /dev/null 2>&1; then
        input_model_dir=$(jq -r '.simulation_generation.input_model_dir' "$config")
        input_model_dir="${config_dir}/${input_model_dir}"
        input_model_file="${global_prefix}_obj_${i}.obj"
        input_model_file_path="${input_model_dir}/${input_model_file}"

        if [  -f "$input_model_file_path" ]; then

            rawData_dir=$(jq -r '.simulation_generation.output_rawData_dir' "$config")
            rawData_dir="${config_dir}/${rawData_dir}"
            mkdir -p "$rawData_dir"     

            inputFov=$(jq -r '.simulation_generation.fov // 38' "$config")  # default to 38 if missing
            rawData_file="${global_prefix}_rawData_${i}.h5"
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

    if jq -e '.pixelization_process | length > 0' "$config" > /dev/null 2>&1; then
        input_rawData_dir=$(jq -r '.pixelization_process.input_rawData_dir' "$config")
        input_rawData_dir="${config_dir}/${input_rawData_dir}"
        input_rawData_file="${global_prefix}_rawData_${i}.h5"
        input_rawData_file_path="${input_rawData_dir}/${input_rawData_file}"
        if [ -f "$input_rawData_file_path" ]; then
            output_pixelized_dir=$(jq -r '.pixelization_process.output_pixelized_dir' "$config")
            output_pixelized_dir="${config_dir}/${output_pixelized_dir}"
            mkdir -p "$output_pixelized_dir"

            output_pixelized_file="${global_prefix}_pixelized_${i}.h5"
            output_pixelized_file_path="${output_pixelized_dir}/${output_pixelized_file}"

            python3 pythonScripts/pixelation.py \
                --input "${input_rawData_file_path}" \
                --output "${output_pixelized_file_path}" || {
                    echo "Error running pixelize.py on ${input_rawData_file_path}"
                    exit 1
                }
        else
            echo "Skipping pixelization: $input_rawData_file_path does not exist"
        fi
    fi

    if jq -e '.imageFormation_process | length > 0' "$config" > /dev/null 2>&1; then
        input_pixelized_dir=$(jq -r '.imageFormation_process.input_pixelized_dir' "$config")
        input_pixelized_dir="${config_dir}/${input_pixelized_dir}"
        input_pixelized_file="${global_prefix}_pixelized_${i}.h5"
        pixelized_file_path="${input_pixelized_dir}/${input_pixelized_file}"

        if [ -f "$pixelized_file_path" ]; then
            depthImage_dir=$(jq -r '.imageFormation_process.output_depthImage_dir' "$config")
            histogram_dir=$(jq -r '.imageFormation_process.output_histogram_dir' "$config")

            depthImage_dir="${config_dir}/${depthImage_dir}"
            histogram_dir="${config_dir}/${histogram_dir}"
            mkdir -p "$depthImage_dir"
            mkdir -p "$histogram_dir"

            depthImage_file="${global_prefix}_depth${i}.png"
            depthImage_file_path="${depthImage_dir}/${depthImage_file}"

            histogram_file="${global_prefix}_histogram_${i}.h5"
            histogram_file_path="${histogram_dir}/${histogram_file}"

            python3 pythonScripts/fromImage.py \
                --input_file "${pixelized_file_path}" \
                --output_image "${depthImage_file_path}" \
                --output_file "${histogram_file_path}" || {
                    echo "Error running fromImage.py on ${pixelized_file_path}"
                    exit 1
                }
        else
            echo "Skipping image formation: $pixelized_file_path does not exist"
        fi
    fi



done
