# Create the build directory and execute CMake.
cd "$(dirname "$0")" &&
cd .. &&
mkdir build &&
cd build &&
../cmake/bin/cmake -G Ninja -DCMAKE_MAKE_PROGRAM=../ninja/ninja ../llvm-project/llvm -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" -DLLVM_BUILD_TESTS=ON

