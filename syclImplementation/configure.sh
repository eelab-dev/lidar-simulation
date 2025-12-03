#! /bin/bash
export ONEAPI_ROOT="$HOME/intel/oneapi"
export CUDA_HOME="$HOME/sw/cuda-12.0"
export PATH="$CUDA_HOME/bin:$PATH"
export LD_LIBRARY_PATH="$CUDA_HOME/lib64:$LD_LIBRARY_PATH"
source "$ONEAPI_ROOT/setvars.sh" --include-intel-llvm > /dev/null
cmake -S . -B build -DENABLE_GPGPU=ON -DCMAKE_C_COMPILER=icx -DCMAKE_CXX_COMPILER=icpx;
