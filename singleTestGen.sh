
#!/bin/bash

# Set execution directory (the directory where this script is located)
SCRIPT_DIR=$(dirname "$(realpath "$0")")
cd "$SCRIPT_DIR" || exit 1

# Source Intel oneAPI environment
. /opt/intel/oneapi/setvars.sh --include-intel-llvm > /dev/null 2>&1

# Activate virtual environment
source .venv/bin/activate


dataSet_dir="singTest" 
mkdir -p "${dataSet_dir}"
model_file_path="positive/model/positive_obj_1.obj"
globalPrefix="single"
rawData_file_path="${dataSet_dir}/${globalPrefix}_rawData.h5"
pixelized_file_path="${dataSet_dir}/${globalPrefix}_pixelized.h5"
histogram_file_path="${dataSet_dir}/${globalPrefix}_histogram.h5"
depthImage_file_path="${dataSet_dir}/${globalPrefix}_depth.png"

./syclImplementation/build/LiDARSimulation \
 --model "${model_file_path}" \
 --output "${rawData_file_path}"\
 --seed 321 || { 
echo "Error running simulation"; 
exit 1;} 

python3 pythonScripts/pixelation.py \
 --input_file "${rawData_file_path}" \
 --output_file "${pixelized_file_path}"|| {
echo "Error running lens.py"; 
exit 1; }


python3 pythonScripts/fromImage.py \
 --input_file "${pixelized_file_path}"\
 --output_image "${depthImage_file_path}"\
 --output_file "${histogram_file_path}" || {
echo "Error running fromImage.py"; 
exit 1;}

