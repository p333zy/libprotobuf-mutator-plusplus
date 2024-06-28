# Info

- Better bindings for libprotobuf-mutator with AFL++:
  - Adds a tree trimming algorithm which visits protobuf nodes and strings
  - Implements sensible defaults for the mutator, including when to combine inputs
- Significantly improves both the quality of generated test cases and performance
  - Nodes that do not produce new coverage will be trimmed
  - This can lead to a substantial reduction in the size of test cases 
- Using the benchmark in the project, the performance impact is over 100% after an hour of fuzzing
  - Typically libprotobuf-mutator performance degrades over time
  - On my laptop the benchmarks are :
    - ~3000 exec/s for libprotobuf-mutator with libfuzzer
    - ~8000 exec/s for lpm++ with afl++
- To get the most out of lpm++, ensure that stability is as high as possible

# ToDo 

- [ ] Add support for trimming UTF-8 Strings (at the moment use the `bytes` type rather than `string`)
- [ ] Improve libprotobuf-mutator string mutation
- [ ] Remove field deletion from libprotobuf-mutator 
- [ ] Research whether adding support for splicing dictionary items improves string input quality
- [ ] Investigate compatibility issue with cmplog instrumentation

# How to Use

1. To build the mutator
  - Requires AFL development headers
  - Either install them in system path
  - Or pull AFL++ source and create a symlink in benchmark/include

```
AFLPATH="/path/to/AFLplusplus/include"
ln -s $AFLPATH ./include/afl
```

2. Pull submodules:

```
git submodule update --recursive 
```

3. Add the following include directories to your build:

```
./
./include
./libprotobuf-mutator
```

4. Add the following files to your project build:

```
trimming.cc
statistics.cc
utils.cc
libprotobuf-mutator/src/binary_format.cc,
libprotobuf-mutator/src/mutator.cc,
libprotobuf-mutator/src/text_format.cc,
libprotobuf-mutator/src/utf8_fix.cc,
```

*For a full example, see the benchmark/ directory*

# Benchmark

- Benchmark uses [nlohmann/json](https://github.com/nlohmann/json) v3.11.3 for JSON parsing

## Build

- To build the mutator:

```
cd benchmark/mutator
meson setup build
cd build
meson compile
```

- To build the harnesses

```
CC=afl-cc CXX=afl-c++ ./build.sh
```

## Running the Fuzzers

- libprotobuf-mutator:

```
mkdir out/libfuzzer
cp ./corpus/vuln1/proto/* ./out/libfuzzer
./build-libfuzzer/fuzz_vuln1 -close_fd_mask=3 -use_value_profile=1 ./out/libfuzzer
```

- libprotobuf-mutator++

```
export AFL_POST_PROCESS_KEEP_ORIGINAL=1
export AFL_CUSTOM_MUTATOR_ONLY=1
export AFL_CUSTOM_MUTATOR_LIBRARY="$PWD/mutator/build/libvuln_mutator.so"

afl-fuzz \
  -i corpus/vuln1/proto \
  -o out/lpm++ \
  -a text \
  -m none \
  -M main \
  -p explore \
  ./build-afl++/fuzz_vuln1
```
