# Projeto com Clang Matchers

To use this plugin you have to follow this steps:

#### Step 0: Obtaining Clang
```
mkdir ~/clang-llvm  
cd ~/clang-llvm  
git clone http://llvm.org/git/llvm.git  
cd llvm/tools  
git clone http://llvm.org/git/clang.git  
cd clang/tools  
git clone http://llvm.org/git/clang-tools-extra.git extra  
```

#### Step 1: Obtain CMake
```
sudo apt-get install cmake
sudo apt-get install cmake-curses-gui
```

#### Step 3: Build Clang
```
cd ~/clang-llvm
mkdir build
cd build
```

- edit CMakeLists.txt from clang-llvm/llvm/  
- line 139 changed to: set(PROJ_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tools/${proj}")  

```
cmake -G "Unix Makefiles" ../llvm -DLLVM_ENABLE_PROJECTS=clang -DLLVM_BUILD_TESTS=ON  # Enable tests; default is off.
make
make check       # Test LLVM only.
make clang-test  # Test Clang only.
make install
```
**NOTE**: (process consumed more than 14GB RAM at some points, so it's recommended to have high amount of RAM and/or virtual memory)

#### Step 4: Set Clang as your compiler

```
cd ~/clang-llvm/build
ccmake ../llvm
```

- The second command will bring up a GUI for configuring Clang. You need to set the entry for CMAKE_CXX_COMPILER.  
- Press 't' to turn on advanced mode. Scroll down to CMAKE_CXX_COMPILER, and set it to /usr/bin/clang++, or wherever you installed it.  
- Press 'c' to configure, then 'g' to generate CMake’s files.  

#### Step 5: Add our plugin to Clang

```
cd ~/clang-llvm/llvm/tools/clang
mkdir tools/extra/reduction
echo 'add_subdirectory(reduction)' >> tools/extra/CMakeLists.txt
```
- edit tools/extra/reduction/CMakeLists.txt

- CMakeLists.txt should have the following contents:

```
set(LLVM_LINK_COMPONENTS support)

add_clang_executable(reduction
  reduction.cpp
)
target_link_libraries(reduction
  PRIVATE
  clangTooling
  clangBasic
  clangASTMatchers
)
```

#### Step 6: Run our plugin

```
cd ~/clang-llvm/build
make
./bin/reduction your-test-file.cpp --
```

**And this is it, this should work!!**
