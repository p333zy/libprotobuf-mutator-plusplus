#!/bin/sh

if [[ -z "$CC" ]]; then
  echo "Please provide CC=afl-cc"
  exit 1
fi

if [[ -z "$CXX" ]]; then 
  echo "Please provide CXX=afl-c++"
  exit 1
fi

meson setup -Dmode=afl build-afl
cd build-afl
meson compile 
cd ../

export AFL_LLVM_LAF_ALL=1
meson setup -Dmode=afl build-afl-laf
cd build-afl-laf
meson compile
unset AFL_LLVM_LAF_ALL
cd ../

export AFL_LLVM_CMPLOG=1
meson setup -Dmode=afl build-afl-cmplog
cd build-afl-cmplog
meson compile
cd ../

export CC=clang
export CXX=clang++
meson setup -Dmode=libfuzzer build-libfuzzer
cd build-libfuzzer
meson compile
cd ../

mkdir out/afl
mkdir out/afl-cmplog
mkdir out/afl-laf
mkdir out/lpm