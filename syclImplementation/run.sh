#! /bin/sh
. /opt/intel/oneapi/setvars.sh --include-intel-llvm > /dev/null;
cd build ; ./HELLOEMBREE /home/peizhao/lidarsimulation/genScene/ cornell_box.obj ../../genScene/positiveResult/outputtest1.txt
