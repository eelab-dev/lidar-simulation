#! /bin/sh
. /opt/intel/oneapi/setvars.sh --include-intel-llvm > /dev/null; 
cmake -S . -B debugBuild -DENABLE_DEBUG=ON -DENABLE_SYCL=ON -DENABLE_GPGPU=OFF;
make -C debugBuild