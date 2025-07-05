#!/bin/bash

# Set execution directory (the directory where this script is located)
SCRIPT_DIR=$(dirname "$(realpath "$0")")
cd "$SCRIPT_DIR" || exit 1

# Source Intel oneAPI environment
. /opt/intel/oneapi/setvars.sh --include-intel-llvm > /dev/null 2>&1

# Activate virtual environment
source .venv/bin/activate


config="animation/animation_config.json"
config_dir=$(dirname "$config")

endIndex=$(jq -r '.global_settings.end_index// empty' "$config")
global_prefix=$(jq -r '.global_settings.global_prefix // empty' "$config")
detector_distance=$(jq -r '.global_settings.detector_distance// empty' "$config") 
inputFov=$(jq -r '.global_settings.fov// empty' "$config")
startIndex=$(jq -r '.global_settings.start_index// empty' "$config")



if [ -z "$global_prefix" ]; then
    echo "'global_prefix' not found."
    exit 1;
fi

if [ -z "$startIndex" ]; then
    echo "'startIndex' not found."
    exit 1;
fi

if [ -z "$endIndex" ]; then
    echo "'endIndex' not found."
    exit 1;
fi

for ((i=startIndex;i<=endIndex+ iteration;i++))
do
    # sleep 2m

    if jq -e '.model_generation | length > 0' "$config" > /dev/null 2>&1; then
        model_dir=$(jq -r '.model_generation.output_model_dir // "model"' "$config")
        model_dir="${config_dir}/${model_dir}"
        mkdir -p "$model_dir"
        model_file="${global_prefix}_obj_${i}.obj"
        model_file_path="${model_dir}/${model_file}"

        # ✅ Initialize simulation_flag with a leading space
        simulation_flag=""

        if [ -n "$detector_distance" ]; then
            simulation_flag+=" --detectorDistance ${detector_distance}"
        fi

        echo $simulation_flag
        python3 pythonScripts/scenGen.py \
        --output_file "${model_file_path}" \
        $simulation_flag || { 
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
        if jq -e '.simulation_generation.static_model' "$config" > /dev/null 2>&1; then
            input_model_file=$(jq -r '.simulation_generation.static_model' "$config")
        fi
        
        
        input_model_file_path="${input_model_dir}/${input_model_file}"

        if [  -f "$input_model_file_path" ]; then

            rawData_dir=$(jq -r '.simulation_generation.output_rawData_dir // "rawData"' "$config")
            rawData_dir="${config_dir}/${rawData_dir}"
            mkdir -p "$rawData_dir"     

            rawData_file="${global_prefix}_rawData_${i}.h5"
            rawData_file_path="${rawData_dir}/${rawData_file}"



            # ✅ Initialize simulation_flag with a leading space
            simulation_flag=""

            # --- NEW LOGIC: Read camera settings from external file ---
            if jq -e '.simulation_generation.camera_config' "$config" > /dev/null 2>&1; then

                # Try to get camera_config_prefix, fallback empty if not found
                camera_config_prefix=$(jq -r '.simulation_generation.camera_config.camera_config_prefix // empty' "$config")

                if [ -n "$camera_config_prefix" ]; then
                    # If prefix is found, construct camera_config with prefix and index
                    camera_config="${camera_config_prefix}_${i}.json"
                else
                    # Otherwise, use camera_config_file from config
                    camera_config=$(jq -r '.simulation_generation.camera_config.camera_config_file' "$config")
                fi           
                # Check if camera_config is still empty, warn user

                if [ -z "$camera_config" ]; then
                    echo "[WARNING] camera_config not found in config file. Please check .simulation_generation.camera_config."
                fi
                full_camera_config_path="${input_model_dir}/${camera_config}"

                echo "INFO: Reading camera setup from ${full_camera_config_path}"
                if [ -f "$full_camera_config_path" ]; then
                    # Parse the camera position and look at point from the camera config file
                    cam_pos_x=$(jq -r '.camera_position[0]' "$full_camera_config_path")
                    cam_pos_y=$(jq -r '.camera_position[1]' "$full_camera_config_path")
                    cam_pos_z=$(jq -r '.camera_position[2]' "$full_camera_config_path")

                    look_at_x=$(jq -r '.look_at_point[0]' "$full_camera_config_path")
                    look_at_y=$(jq -r '.look_at_point[1]' "$full_camera_config_path")
                    look_at_z=$(jq -r '.look_at_point[2]' "$full_camera_config_path")

                    # Append the new flags to the simulation_flag variable
                    simulation_flag+=" --cameraPosition ${cam_pos_x} ${cam_pos_y} ${cam_pos_z}"
                    simulation_flag+=" --lookAt ${look_at_x} ${look_at_y} ${look_at_z}"
                else
                    echo "WARNING: camera_config_file specified but not found at '${full_camera_config_path}'. Skipping camera override."
                fi
            fi


            # ✅ Append flags conditionally
            
            if jq -e '.simulation_generation.ssp' "$config" > /dev/null 2>&1; then
                ssp=$(jq -r '.simulation_generation.ssp' "$config")
                simulation_flag+=" --ssp ${ssp}"
            fi

            if jq -e '.simulation_generation.width' "$config" > /dev/null 2>&1; then
                width=$(jq -r '.simulation_generation.width' "$config")
                simulation_flag+=" --width ${width}"
            fi

            if jq -e '.simulation_generation.height' "$config" > /dev/null 2>&1; then
                height=$(jq -r '.simulation_generation.height' "$config")
                simulation_flag+=" --height ${height}"
            fi

            if [ -n "$detector_distance" ]; then
                simulation_flag+=" --detectorDistance ${detector_distance}"
            fi

            if [ -n "$inputFov" ]; then
                simulation_flag+=" --fov ${inputFov}"
            fi
            echo $simulation_flag

            ./syclImplementation/build/LiDARSimulation \
            --model "${input_model_file_path}" \
            --output "${rawData_file_path}" \
            --seed 4 \
            $simulation_flag || { 
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
            output_pixelized_dir=$(jq -r '.pixelization_process.output_pixelized_dir // "pixelizedData"' "$config")
            output_pixelized_dir="${config_dir}/${output_pixelized_dir}"
            mkdir -p "$output_pixelized_dir"

            output_pixelized_file="${global_prefix}_pixelized_${i}.h5"
            output_pixelized_file_path="${output_pixelized_dir}/${output_pixelized_file}"

            # ✅ Initialize simulation_flag with a leading space
            simulation_flag=""

            if [ -n "$inputFov" ]; then
                simulation_flag+=" --fov ${inputFov}"
            fi

            if jq -e '.pixelization_process.height' "$config" > /dev/null 2>&1; then
                height=$(jq -r '.pixelization_process.height' "$config")
                simulation_flag+=" --image_height ${height}"
            fi
            
            if jq -e '.pixelization_process.width' "$config" > /dev/null 2>&1; then
                width=$(jq -r '.pixelization_process.width' "$config")
                simulation_flag+=" --image_width ${width}"
            fi

            echo $simulation_flag

            ./rustLib/release/lidar_pixelation \
                --input_file "${input_rawData_file_path}" \
                -o "${output_pixelized_file_path}"\
                $simulation_flag || {
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
            depthImage_dir=$(jq -r '.imageFormation_process.output_depthImage_dir // "depthImage"' "$config")
            histogram_dir=$(jq -r '.imageFormation_process.output_histogram_dir // "histogramData"' "$config")

            depthImage_dir="${config_dir}/${depthImage_dir}"
            histogram_dir="${config_dir}/${histogram_dir}"
            mkdir -p "$depthImage_dir"
            mkdir -p "$histogram_dir"

            # ✅ Initialize simulation_flag with a leading space
            simulation_flag=""


            # Append required input
            simulation_flag+=" --input_file ${pixelized_file_path}"


            # Construct output path and add to simulation_flag
            histogram_file="${global_prefix}_histogram_${i}.h5"
            histogram_file_path="${histogram_dir}/${histogram_file}"
            simulation_flag+=" --output_file ${histogram_file_path}"
            

            # Build output path and update simulation_flag
            depthImage_file="${global_prefix}_depth${i}.png"
            depthImage_file_path="${depthImage_dir}/${depthImage_file}"
            simulation_flag+=" --output_image ${depthImage_file_path}"

            if jq -e '.imageFormation_process.bin_number' "$config" > /dev/null 2>&1; then
                bin_number=$(jq -r '.imageFormation_process.bin_number' "$config")
                simulation_flag+=" --bin_number ${bin_number}"
            fi

            if jq -e '.imageFormation_process.min_range' "$config" > /dev/null 2>&1; then
                min_range=$(jq -r '.imageFormation_process.min_range' "$config")
                simulation_flag+=" --min_range ${min_range}"
            fi

            if jq -e '.imageFormation_process.bin_width' "$config" > /dev/null 2>&1; then
                bin_width=$(jq -r '.imageFormation_process.bin_width' "$config")
                simulation_flag+=" --bin_width ${bin_width}"
            fi

            echo $simulation_flag
            python3 pythonScripts/fromImage.py $simulation_flag || {
                    echo "Error running fromImage.py on ${pixelized_file_path}"
                    exit 1
                }
        else
            echo "Skipping image formation: $pixelized_file_path does not exist"
        fi
    fi



done
