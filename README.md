<!-- TODO: explain here what reduction-detector does, including the detailed algorithm it implements. A link to an article probably would be very useful. -->

This is an application of Clang’s LibTooling API. See [here][1] for a tutorial and [here][2] for full documentation.

[1]: <https://clang.llvm.org/docs/LibASTMatchersTutorial.html>
[2]: <https://clang.llvm.org/docs/index.html#using-clang-as-a-library>


The core of the program is the subsystem in `src/loop_analysis`, which provides an AST matcher and an implementation of MatchCallback that can be used to detect reductions. Those can be used as part of a larger program. Its interface is described in [loop_analysis.h][3].

[3]: <src/loop_analysis/loop_analysis.h>

Identification and filtering of "trivial reductions" (that is: reductions that also are detected by Cetus) isn't very reliable. It's a good approximation, but don't trust it blindly.
See the related to-do items in the to-do list at [reduction-detector.cpp][4].

[4]: <src/reduction-detector.cpp>

***

# Build

On Ubuntu 20.04, you might first need to execute
<pre><code>sudo apt install <a href="https://packages.ubuntu.com/focal/zlib1g-dev">zlib1g-dev</a></code></pre>

reduction-detector links against LLVM 12.0.0. Download [a build of LLVM 12.0.0][3], change the value of `LLVM_PATH` in our makefile accordingly, and then run `make`. The program will be built as `build/reduction-detector`.

[3]: <https://github.com/llvm/llvm-project/releases/tag/llvmorg-12.0.0#:~:text=566%20Bytes-,clang%2Bllvm-12.0.0-x86_64-linux-gnu-ubuntu-20.04.tar.xz,-432%20MB>

***

# Usage

You can invoke reduction-detector on an individual C file or on a tree with multiple C files. For example:
```
build/reduction-detector "test cases"
```

Use `--help` to see other possible arguments.

Like other Clang-based tools, reduction-detector benefits greatly from a JSON [compilation database][4] describing the program under analysis (including, for example, the location of its header files). Without one, you'll probably see many parsing errors and receive very incomplete output. See [this][5] and [this][6].

[4]: <https://eli.thegreenplace.net/2014/05/21/compilation-databases-for-clang-based-tools> (Eli Bendersky's very good introduction to compilation databases)
[5]: <https://clang.llvm.org/docs/JSONCompilationDatabase.html> (Official Clang documentation)
[6]: <https://sarcasm.github.io/notes/dev/compilation-database.html> (Other good stuff I found on Google)

By default, reduction-detector writes its output to `detected_reductions.out`.

***

# FAQ
 
### Why does reduction-detector complain that it can't find header files included by the C files being analyzed?
This is usually caused by a lack of a compilation database. To analyse a C program, reduction-detector needs to be able to find header files `#include`d by it. (Clang's parser reasonably aborts with an error message if it can't find a header file `#include`d by a C translation unit.) If those header files aren't in the same directories as the source files that include them (for example, if they are in a separate `include` directory), you'll have to either create a compilation database or directly tell reduction-detector where to find them. You can do the latter by using the [`-I`](https://clang.llvm.org/docs/ClangCommandLineReference.html#id8) option after a double dash (`--`). (The arguments passed after `--` are captured by the Clang tooling we use internally.)
 
For example, the following command line could be used to analyze a copy of the source code of [OpenSSL](https://github.com/openssl/openssl) that sits at `../openssl-master`:
```
build/reduction-detector ../openssl-master -- -I ../openssl-master/include
```
 
### I did do that. Why does reduction-detector still complain that it can't find some standard header files (for example, <stddef.h>)?
It might not be looking in the right places. If your system compiler _does_ find those standard header files when you compile stuff, it might help to make reduction-detector search for header files in the same places as your system compiler. If you execute `gcc -x c - -E -v <<< ""`, GCC will, among other things, tell you the directories in which it looks for system header files. (You can replace `gcc` with `clang`, if that's what you use.)
