<!-- TODO: explain here what reduction-detector does, including the detailed algorithm it implements. A link to an article probably would be very useful. -->

This is an application of Clang’s LibTooling API. See [here][1] for a tutorial and [here][2] for full documentation.

[1]: <https://clang.llvm.org/docs/LibASTMatchersTutorial.html>
[2]: <https://clang.llvm.org/docs/index.html#using-clang-as-a-library>


The core of the program is the subsystem in `src/loop_analysis`, which provides an AST matcher and an implementation of MatchCallback that can be used to detect reductions. Those can be used as part of a larger program. Its interface is described in [loop_analysis.h][3].

[3]: <src/loop_analysis/loop_analysis.h>

Identification and filtering of "trivial reductions" (that is: reductions that also are detected by Cetus) isn't very reliable. It's a good approximation, but don't trust it blindly.
See the related to-do items in the to-do list at [reduction-detector.cpp][4].

[4]: <src/reduction-detector.cpp>


As a rudimentary form of regression testing, we watch closely changes in the output of the tests run by `test.sh`. After making any changes to the program, before committing, we run `test.sh` and, by using `git diff`, we ensure that all changes introduced in the output files are intended (or that unintended results that should be fixed later are properly documented).

***

# Build

On Ubuntu 20.04, you might first need to execute
<pre><code>sudo apt install <a href="https://packages.ubuntu.com/focal/zlib1g-dev">zlib1g-dev</a></code></pre>

reduction-detector links against LLVM 12.0.0. Download [a build of LLVM 12.0.0][5], change the value of `LLVM_PATH` in our makefile accordingly, and then run `make`. The program will be built as a single, statically-linked executable file `build/reduction-detector`.

[5]: <https://github.com/llvm/llvm-project/releases/tag/llvmorg-12.0.0#:~:text=566%20Bytes-,clang%2Bllvm-12.0.0-x86_64-linux-gnu-ubuntu-20.04.tar.xz,-432%20MB>


To use reduction-detector, you should also have Clang 12 installed[^1]. After building reduction-detector, copy it into the same directory where Clang resides (which is usually `/usr/bin`). This can be done with the following command line:
```Bash
sudo cp build/reduction-detector "$(dirname "$(type -p clang)")"
```
(Besides putting reduction-detector in your PATH, this also is important for reasons discussed [here][5.5].) Then, you can invoke the program as `reduction-detector`.

[^1]: It is indeed important to have Clang 12 specifically, which can be installed with `sudo apt install clang-12` on Ubuntu. This is because our tool looks for system header files in `/usr/lib/clang/12.0.0/include`.

[5.5]: <https://clang.llvm.org/docs/LibTooling.html#builtin-includes>


***

# Usage

You can invoke reduction-detector on an individual C file or on a tree with multiple C files. For example:
```Bash
reduction-detector "test cases"
```

Use `--help` to see other possible arguments.

Like other Clang-based tools, reduction-detector benefits greatly from a JSON [compilation database][6] describing the program under analysis (including, for example, the location of its header files). Without one, you'll probably see many parsing errors and receive very incomplete output. See [this][7] and [this][8].

[6]: <https://eli.thegreenplace.net/2014/05/21/compilation-databases-for-clang-based-tools> (Eli Bendersky's very good introduction to compilation databases)
[7]: <https://clang.llvm.org/docs/JSONCompilationDatabase.html> (Official Clang documentation)
[8]: <https://sarcasm.github.io/notes/dev/compilation-database.html> (Other good stuff I found on Google)

By default, reduction-detector writes its output to `detected_reductions.out`.

***

# Referencing reduction-detector

In scientific works that involve usage of reduction-detector, we ask that the following article be referenced:

> João Rezende, Edevaldo Santos, and Gerson Cavalheiro. 2021. Detecção de operações de redução em programas C. In Anais do XXII Simpósio em Sistemas Computacionais de Alto Desempenho, outubro 26, 2021, Belo Horizonte/MG, Brasil. SBC, Porto Alegre, Brasil, 204-215.

For other citation styles or BibTeX meta-data, see [here](https://sol.sbc.org.br/index.php/wscad/article/view/18524).

***

# FAQ
 
<details><summary>Why does reduction-detector complain that it can't find header files included by the C files being analyzed?</summary>
This is usually caused by a lack of a compilation database. To analyze a C program, reduction-detector needs to be able to find header files `#include`d by it. (Clang's parser reasonably aborts with an error message if it can't find a header file `#include`d by a C translation unit.) If those header files aren't in the same directories as the source files that include them (for example, if they are in a separate `include` directory), you'll have to either create a compilation database or directly tell reduction-detector where to find them. You can do the latter by using the [`-I`](https://clang.llvm.org/docs/ClangCommandLineReference.html#include-path-management) option after a double dash (`--`). (The arguments passed after `--` are captured by the Clang tooling we use internally.)
 
For example, the following command line could be used to analyze a copy of the source code of [OpenSSL](https://github.com/openssl/openssl) that sits at `../openssl-master`:
```
build/reduction-detector ../openssl-master -- -I ../openssl-master/include
```
</details>
 
<details><summary>I did do that. Why does reduction-detector still complain that it can't find some standard header files (for example, <stddef.h>)?</summary>
This probably is a consequence of not having Clang 12 installed, or of not having copied reduction-detector into the right place, as described above. If you execute `reduction-detector <an arbitrary C file> -- -v`, reduction-detector will, among other things, tell you the directories in which it looks for system header files. That list should include a directory like `/usr/lib/clang/12.0.0/include`. If it doesn't, then you might not have copied reduction-detector into the right place. If reduction-detector complains that such a directory doesn't exist, then you might not have the right version of Clang installed.
</details>