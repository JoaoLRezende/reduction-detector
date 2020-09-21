### Setup

This code expects to sit alongside the source tree of LLVM and Clang with clang-tools-extra, organized as follows:

```
finding-parallelizable-code-syntactically
├── ... project files ...
└── llvm
    ├── ... LLVM files ...
    └── tools
        ├── ... LLVM tools ...
        └── clang
            ├── ... Clang files ...
            └── tools
                ├── ... Clang tools ...
                └── extra
                    └── ... clang-tools-extra files ...
```

To set this up, build everything and run tests:

#### Step 0: Obtain Clang
```
cd finding-parallelizable-code-syntactically
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
In the file `/llvm/CMakeLists.txt`, change the line
```
set(PROJ_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../${proj}")
```
to
```
set(PROJ_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tools/${proj}")  
```
Then:
```
cd finding-parallelizable-code-syntactically
mkdir build
cd build
```

```
cmake -G "Unix Makefiles" ../llvm -DLLVM_ENABLE_PROJECTS=clang -DLLVM_BUILD_TESTS=ON  # Enable tests; default is off.
make
make check       # Test LLVM only.
make clang-test  # Test Clang only.
make install
```
**NOTE**: this process consumed more than 14 GB RAM at some points.

#### Step 4: Set Clang as your compiler

```
cd finding-parallelizable-code-syntactically/build
ccmake ../llvm
```

- The second command will bring up a GUI for configuring Clang. You need to set the entry for CMAKE_CXX_COMPILER.  
- Press 't' to turn on advanced mode. Scroll down to CMAKE_CXX_COMPILER, and set it to /usr/bin/clang++, or wherever you installed it.  
- Press 'c' to configure, then 'g' to generate CMake’s files.  

#### Step 5: Add our plugin to Clang

```
cd finding-parallelizable-code-syntactically/llvm/tools/clang
mkdir tools/extra/reduction
echo 'add_subdirectory(reduction)' >> tools/extra/CMakeLists.txt
```
Edit `tools/extra/reduction/CMakeLists.txt`. It should have the following contents:

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
cd finding-parallelizable-code-syntactically/build
make
./bin/reduction your-test-file.cpp --
```
-------------
### To do
- Make sure the steps in the setup guide above are actually sufficient.
- Describe fully the steps to add new plugin modules or update existing ones. Briefly mention our test scripts and how they are used to quickly update existing modules.
