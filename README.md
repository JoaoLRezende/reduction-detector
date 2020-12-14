This is an application of Clang’s LibTooling API. See [here][1] for a tutorial and [here][2] for full documentation.

[1]: <https://clang.llvm.org/docs/LibASTMatchersTutorial.html>
[2]: <https://clang.llvm.org/docs/index.html#using-clang-as-a-library>

### Setup

To build, you need a copy of LLVM's (and Clang's) source tree within this directory, in a directory named `llvm-project`.
You can set that up by following [these](https://clang.llvm.org/docs/LibASTMatchersTutorial.html#step-0-obtaining-clang) instructions (though in this directory, rather than in `~/clang-llvm`).
You also need to have [Ninja][1] and [CMake][2] installed.

[1]: <https://ninja-build.org/>
[2]: <https://cmake.org/>

Then:
```
cd finding-parallelizable-code-syntactically/llvm-project/clang-tools-extra
mkdir reduction-detector
echo 'add_subdirectory(reduction-detector)' >> CMakeLists.txt
```
Create in that new directory a file named `CMakeLists.txt` with the following content:
```
set(LLVM_LINK_COMPONENTS support)

file(GLOB reduction-detector-SRC
    "*.cpp"
)
 
add_clang_executable(reduction-detector
     ${reduction-detector-SRC}
)
target_link_libraries(reduction-detector
  PRIVATE
  clangTooling
  clangBasic
  clangASTMatchers
)
```
Then, execute our script `set-up-build.sh` to generate the necessary Ninja files with CMake. Then, `build.sh` can be used to copy the tool's source files into LLVM's tree and build it.
