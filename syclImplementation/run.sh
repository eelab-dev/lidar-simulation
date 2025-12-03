#! /bin/sh

export ONEAPI_ROOT="$HOME/intel/oneapi"
source "$ONEAPI_ROOT/setvars.sh" --include-intel-llvm > /dev/null
# . /opt/intel/oneapi/setvars.sh --include-intel-llvm > /dev/null;
cd build ; ./HELLOEMBREE /home/peizhao/lidarsimulation/genScene/ cornell_box.obj ../../genScene/positiveResult/outputtest1.txt
