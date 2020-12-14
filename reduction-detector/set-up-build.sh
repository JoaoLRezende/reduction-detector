# Create the build directory, copy our source files into LLVM's source tree and execute CMake.
cd "$(dirname "$0")"

rm -f           ../llvm-project/clang-tools-extra/reduction-detector/*.cpp &&
cp -p src/*.cpp ../llvm-project/clang-tools-extra/reduction-detector/ &&
cd .. &&
mkdir -p build &&
cd build &&
cmake -G Ninja ../llvm-project/llvm -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" -DLLVM_BUILD_TESTS=ON &&
mkdir -p ../llvm-project/clang/include/reduction-detector/
