# Actul -- Automated concurreny testing and scheduling tool
Actul automatically instruments Posix parallel C/C++ programs and runs concurrency checks. Therefore the instrumented program is executed several times using process forks.

## Tested Environments
Actul was successfully tested on Ubuntu 16.04, compiled with GCC 5.4. Currently Actul does not build with the Clang compiler!

## Software dependencies
	CMake (>= 3.1.0)
	C++11 compatible compiler
	LLVM libraries with Clang (>= 4.0, < 5.0)

On some server systems we discovered some issues with Release-Builds. To be safe, build Actul and Clang/LLVM with -DCMAKE_BUILD_TYPE="Debug"
	
## Installation: LLVM/Clang instrumentation build:
The LLVM/Clang build creates wrappers which automatically instrument the compiled programs. This build depends on a complete llvm and clang build. For further instruction see llvm.org/docs/GettingStarted.html.

```
git clone https://git.zib.de/hartung/Actul.git
cd Actul
mkdir build
cd build
cmake ..
make install
```
If cmake couldn't find llvm/clang or a specific version should be used, add the flag `-DLLVM_INSTALL_DIR=/path/to/llvm-install-dir`.


## Usage
The tool provides a C++-Compiler (`actul++`) and C-Compiler (`actul`), which are wrappers of the given Clang compiler and a linker (`actuld`), which is a wrapper of the systems linker.
A simple C++ application "simple.cpp" can be compiled and ran similar to a normal compilation/exection:

```
actul++ simple.cpp -o simple -pthread
./simple
```

## Runtime Settings

Several settings are read by actul during program start. The following environment variables are supported:


| Environment Variable			| Description	|
| ----------------------------- | ------------- |
| ACTUL_ONLY_PERMUTATION		| Permutes every data race once. Requires less test cases than the normal configuration, 0 is off, 1 is on (default=0) |
| ACTUL_ONLY_RANDOM				| Executes ´ACTUL_MIN_NUM_TESTS´ tests with random scheduling, 0 is off, 1 is on (default=0)|
| ACTUL_VERBOSITY				| Detail of output. 0 prints only testing statistics, 1 prints all data race pathes, 2 prints additional information about single tests (default=1)|
| ACTUL_MAX_NUM_TESTS			| Maximal number of executed test (default=4294967295) |
| ACTUL_MIN_NUM_TESTS			| Maximal number of executed test (default=1) |
| ACTUL_NUM_WORKERS				| Number of parallel executed tests (default=1) |
| ACTUL_MAX_MEM_MB				| Maximal allocated memory in MB (default=32000) |
| ACTUL_SEED					| Start seed for random scheduling. Change this value to explore different schedules (default=1) |
| ACTUL_MAX_TIME				| Maximal allowed runtime in seconds. Will abort and print results, when time is over (default=3600) |
| ACTUL_SUCCESSIVE_RELEASES		| Maximal number of successive releases of the same thread. A higher number allows the scheduler to let single threads inactive for a longer time (default=20) |
| ACTUL_MAX_SEQUENTIAL_READS	| Number of successive reads which are allowed, without calling the scheduler. Prevents deadlocking on while loops which have no progress without calling other threads|
| ACTUL_IDLE_MICROSECONDS		| Defines the number of microseconds the master process will idle, when no test finished. Increase this number, when the tested application has a high workload (default=20) |
| ACTUL_NUM_SYSTEMATIC			| Determines how many data races are tested in the same context. Increase if data races could not be classified properly (default=2) |
| ACTUL_MAX_CHECKED_DATARACES	| Restricts the number of tested data races. Currently there is no selection progress. First detected data races are tested (default=10000) |
| ACTUL_PRINT_INTERVAL			| Time inbetween status outputs of actul |

# Acknowledgement

Actul was created in the [HPSV project](http://www.zib.de/projects/hpsv-highly-parallel-software-verification-concurrent-applications-automotive-industry), which was funded by Federal Ministry of Education and Research.