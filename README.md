This is an application of Clang’s LibTooling API. See [here][1] for a tutorial and [here][2] for full documentation.

[1]: <https://clang.llvm.org/docs/LibASTMatchersTutorial.html>
[2]: <https://clang.llvm.org/docs/index.html#using-clang-as-a-library>

# Setup

This program links against LLVM 9.0.0. Download a binary distribution of LLVM 9.0.0 from [LLVM's website][3], change the value of `LLVM_PATH` in our makefile accordingly, and then run `make`.

[3]: <https://releases.llvm.org/download.html#9.0.0>
***

# FAQ
 
### Why does reduction-detector complain that it can't find header files included by the C files being analyzed?
To analyse a C program, reduction-detector needs to be able to find header files `#include`d by it. (Clang's parser reasonably aborts with an error message if it can't find a header file `#include`d by a C translation unit.) If those header files aren't in the same directories as the source files that include them (for example, if they are in a separate `include` directory), you have to tell reduction-detector where to find them. You can do that by using the [`-I`](https://clang.llvm.org/docs/ClangCommandLineReference.html#id8) option after a double dash (`--`). (The arguments passed after `--` are captured by the Clang tooling we use internally.)
 
For example, the following command line could be used to analyze a copy of the source code of [OpenSSL](https://github.com/openssl/openssl) that sits at `../openssl-master`:
```
build/reduction-detector ../openssl-master -- -I ../openssl-master/include
```
 
### I did do that. Why does reduction-detector still complain that it can't find some standard header files (for example, <stddef.h>)?
It might not be looking in the right places. If your system compiler _does_ find those standard header files when you compile stuff, it might help to make reduction-detector search for header files in the same places as your system compiler. If you execute `gcc -x c - -E -v <<< ""`, GCC will, among other things, tell you the directories in which it looks for system header files. (You can replace `gcc` with `clang`, if that's what you use.)
