#! /bin/sh
export ONEAPI_ROOT="$HOME/intel/oneapi"
export CUDA_HOME="$HOME/sw/cuda-12.0"
export PATH="$CUDA_HOME/bin:$PATH"
export LD_LIBRARY_PATH="$CUDA_HOME/lib64:$LD_LIBRARY_PATH"
source "$ONEAPI_ROOT/setvars.sh" --include-intel-llvm > /dev/null
cmake -S . -B debugBuild -DENABLE_DEBUG=ON -DENABLE_SYCL=ON -DENABLE_GPGPU=OFF;
make -C debugBuild