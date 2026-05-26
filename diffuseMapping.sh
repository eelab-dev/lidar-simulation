#!/usr/bin/env bash


# oneAPI env scripts may reference unset vars internally; temporarily relax nounset.
export ONEAPI_ROOT="$HOME/intel/oneapi"
source "$ONEAPI_ROOT/setvars.sh" --include-intel-llvm > /dev/null
source ~/miniconda3/etc/profile.d/conda.sh
conda activate lidarSimulation

# Default config. Override by passing first argument.

default_config="general_negative/general_negative_diffuse.json"

config="general_positive/general_positive_diffuse.json"

config="${1:-$default_config}"

# config="general_negative/general_negative_diffuse.json"
config_dir=$(dirname "$config")

if [[ ! -f "$config" ]]; then
    echo "Config not found: $config"
    exit 1
fi

startIndex=$(jq -r '.global_settings.start_index // empty' "$config")
endIndex=$(jq -r '.global_settings.end_index // empty' "$config")
global_prefix=$(jq -r '.global_settings.global_prefix // empty' "$config")
global_fov=$(jq -r '.global_settings.fov // empty' "$config")

if [[ -z "$global_prefix" ]]; then
    echo "'global_prefix' not found."
    exit 1
fi
if [[ -z "$startIndex" ]]; then
    echo "'start_index' not found."
    exit 1
fi
if [[ -z "$endIndex" ]]; then
    echo "'end_index' not found."
    exit 1
fi



for ((i=startIndex; i<=endIndex; i++)); do

    input_model_dir=$(jq -r '.diffuseMap_generation.input_model_dir // "model"' "$config")
    input_model_dir="${config_dir}/${input_model_dir}"

    input_model_file="${global_prefix}_obj_${i}.usda"
    if jq -e ".diffuseMap_generation.static_model" "$config" > /dev/null 2>&1; then
        input_model_file=$(jq -r ".diffuseMap_generation.static_model" "$config")
    fi
    input_model_file_path="${input_model_dir}/${input_model_file}"

    if [[ ! -f "$input_model_file_path" ]]; then
        echo "Skipping diffuse mapping: model not found: $input_model_file_path"
        continue
    fi

    output_dir=$(jq -r ".diffuseMap_generation.output_diffuse_dir // \"diffuseMap\"" "$config")
    output_dir="${config_dir}/${output_dir}"
    mkdir -p "$output_dir"

    output_file="${global_prefix}_diffuse_${i}.h5"
    output_file_path="${output_dir}/${output_file}"

    simulation_flag=""

    # Camera config (support camera_dir and camear_dir typo)
    if jq -e ".diffuseMap_generation.camera_config" "$config" > /dev/null 2>&1; then
        input_camera_dir=$(jq -r '.diffuseMap_generation.camera_config.camera_dir // .diffuseMap_generation.camera_config.camear_dir // "camera"' "$config")
        input_camera_dir="${config_dir}/${input_camera_dir}"

        camera_config_prefix=$(jq -r ".diffuseMap_generation.camera_config.camera_config_prefix // empty" "$config")
        static_camera_config=$(jq -r ".diffuseMap_generation.camera_config.static_camera_config // empty" "$config")

        iter_camera_file="${global_prefix}_${camera_config_prefix}_${i}.json"
        full_camera_config_path=""

        if [[ -n "$camera_config_prefix" && -f "${input_camera_dir}/${iter_camera_file}" ]]; then
            full_camera_config_path="${input_camera_dir}/${iter_camera_file}"
            echo "Using iter camera config: $full_camera_config_path"
        elif [[ -n "$static_camera_config" && -f "${input_camera_dir}/${static_camera_config}" ]]; then
            full_camera_config_path="${input_camera_dir}/${static_camera_config}"
            echo "Using static camera config: $full_camera_config_path"
        fi

        if [[ -n "$full_camera_config_path" ]]; then
            cam_pos_x=$(jq -r '.camera_position[0]' "$full_camera_config_path")
            cam_pos_y=$(jq -r '.camera_position[1]' "$full_camera_config_path")
            cam_pos_z=$(jq -r '.camera_position[2]' "$full_camera_config_path")

            look_at_x=$(jq -r '.look_at_point[0]' "$full_camera_config_path")
            look_at_y=$(jq -r '.look_at_point[1]' "$full_camera_config_path")
            look_at_z=$(jq -r '.look_at_point[2]' "$full_camera_config_path")

            up_dir_x=$(jq -r '.up_direction[0]' "$full_camera_config_path")
            up_dir_y=$(jq -r '.up_direction[1]' "$full_camera_config_path")
            up_dir_z=$(jq -r '.up_direction[2]' "$full_camera_config_path")

            simulation_flag+=" --cameraPosition ${cam_pos_x} ${cam_pos_y} ${cam_pos_z}"
            simulation_flag+=" --lookAt ${look_at_x} ${look_at_y} ${look_at_z}"
            simulation_flag+=" --up ${up_dir_x} ${up_dir_y} ${up_dir_z}"
        else
            echo "No camera override found for index $i; using executable defaults."
        fi
    fi

    if jq -e ".diffuseMap_generation.ssp" "$config" > /dev/null 2>&1; then
        ssp=$(jq -r ".diffuseMap_generation.ssp" "$config")
        simulation_flag+=" --ssp ${ssp}"
    fi
    if jq -e ".diffuseMap_generation.width" "$config" > /dev/null 2>&1; then
        width=$(jq -r ".diffuseMap_generation.width" "$config")
        simulation_flag+=" --width ${width}"
    fi
    if jq -e ".diffuseMap_generation.height" "$config" > /dev/null 2>&1; then
        height=$(jq -r ".diffuseMap_generation.height" "$config")
        simulation_flag+=" --height ${height}"
    fi

    # Prefer explicit fov_x/fov_y in mapping section; fallback to global_settings.fov for both axes.
    if jq -e ".diffuseMap_generation.fov_x" "$config" > /dev/null 2>&1; then
        fov_x=$(jq -r ".diffuseMap_generation.fov_x" "$config")
        simulation_flag+=" --fov_x ${fov_x}"
    elif [[ -n "$global_fov" ]]; then
        simulation_flag+=" --fov_x ${global_fov}"
    fi

    if jq -e ".diffuseMap_generation.fov_y" "$config" > /dev/null 2>&1; then
        fov_y=$(jq -r ".diffuseMap_generation.fov_y" "$config")
        simulation_flag+=" --fov_y ${fov_y}"
    elif [[ -n "$global_fov" ]]; then
        simulation_flag+=" --fov_y ${global_fov}"
    fi

    echo "Running diffuse map for index ${i}"
    echo "Model: ${input_model_file_path}"
    echo "Output: ${output_file_path}"
    echo "Flags:${simulation_flag}"

    ./syclImplementation/build/LiDARSceneDiffuseMap \
        --model "${input_model_file_path}" \
        --output "${output_file_path}" \
        --seed 4 \
        ${simulation_flag} || {
        echo "Error running LiDARSceneDiffuseMap"
        exit 1
    }

    if jq -e '.mv_file | length > 0' "$config" > /dev/null 2>&1; then
        echo "starting copying ....."
        destination_diffuse_dir=$(jq -r '.mv_file.destination_diffuse_dir // empty' "$config")

        if [ -n "$destination_diffuse_dir" ]; then
            source_diffuse_dir=$(jq -r '.diffuseMap_generation.output_diffuse_dir // "diffuseMap"' "$config")
            source_diffuse_dir="${config_dir}/${source_diffuse_dir}"

            destination_diffuse_dir="${destination_diffuse_dir}"
            mkdir -p "$destination_diffuse_dir"

            diffuse_file="${global_prefix}_diffuse_${i}.h5"
            source_diffuse_file_path="${source_diffuse_dir}/${diffuse_file}"
            destination_diffuse_file_path="${destination_diffuse_dir}/${diffuse_file}"

            if [ -f "$source_diffuse_file_path" ]; then
 
                if mv "$source_diffuse_file_path" "$destination_diffuse_file_path"; then
                    echo "Diffuse file moved successfully"
                else
                    echo "Failed to move diffuse file to $destination_diffuse_file_path"
                    exit 1
                fi
            else
                echo "Skipping diffuse file move: $source_diffuse_file_path does not exist"
            fi
        fi
    fi

done
