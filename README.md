# LLForth [![MIT License](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/riywo/llforth/blob/master/LICENSE.txt)
Experimental Forth implementation in LLVM.

## Summary
This software is built for a personal research to understand computer architecture. Impmlementing stack-based programming language([Forth](https://en.wikipedia.org/wiki/Forth_(programming_language))) using a register-based virtual machine ([LLVM](https://en.wikipedia.org/wiki/LLVM)) is useful to know both machine/language architecture.

LLForth uses following technique:

- Restricted static compiler (`llfortc`) from Forth to LLVM Intermediate Representation (LLVM IR) and Full feature interpreter (`llforth`) written in Forth and compiled by `llforthc`
- [Indirect Threaded Code (ITC)](https://en.wikipedia.org/wiki/Threaded_code#Indirect_threading) to implement inner interpreter by LLVM IR
- Naive memory implementation for Stack and Return Stack by LLVM IR
- Partial memory cell for only word definitions excluding string of name of words
- [Foreign Function Interface](https://en.wikipedia.org/wiki/Foreign_function_interface) to delegate platform dependent features (e.g. stdio) to [Rust](https://www.rust-lang.org/) and share it between compiler and interpreter

## Getting started

### Prerequisites (versions are tested by the author)
- CMake (3.13.2)
- LLVM (7.0.0)
- Rust (1.31.1)

### Build
All required steps are declared in `CMakeLists.txt` including compiling Rust library, so you just need to execute CMake build:

```sh
$ mkdir cmake-build-debug
$ cd cmake-build-debug
# If you install LLVM outside default path like Homebrew:
# export LLVM_DIR=/usr/local/opt/llvm/lib/cmake
$ cmake ..
$ make llforth
```

### Execution
`llforth` is statically linked with required libraries except `libc`:

```sh
$ otool -L ./llforth
./llforth:
	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.200.5)
```

Therefore, you can execute it easily:

```sh
$ ./llforth --help
llforth 0.1

USAGE:
    llforth [FILE]

FLAGS:
    -h, --help       Prints help information
    -V, --version    Prints version information

ARGS:
    <FILE>    Source file
```

## Usage
`llforth` can read from both stdin and source file. For example, you can run it interectively powered by [Rustyline](https://crates.io/crates/rustyline/) which is Readline like library: 

```sh
$ ./llforth
> 1 1 + .
2
> : hi ." Hello world!" ;

> hi
Hello world!
```

Also, you can input via stdin non-interectively:

```sh
$ echo '1 2 + .' | ./llforth
3
```

Finally, you can read a source file on your file system:

```sh
$ echo '1 2 + .' > /tmp/test.fs && ./llforth /tmp/test.fs
3
```

## Supported words
See https://github.com/riywo/llforth/wiki/Supported-words

## Architecture
See https://github.com/riywo/llforth/wiki/Architecture

## Acknowledgments

### Low-Level Programming / Forthress
This project is heavily inspired from a book [*"Low-Level Programming"*](https://amzn.to/2BBYugn) which has a great section for implementing Forth using x86_64 assembly only. The code base is also available from the author's repository called [Forthress](https://github.com/sayon/forthress) under MIT License. Most of architectures of LLForth are borrowed from Forthless, for example ITC, Memory cell, etc.

### Series of posts on the evolution of TransForth
On top of assembly implementation mirrored from Forthress, I implemented some forth words *by* forth, for example `if ... else ... then` or `begin ... until`. For those words are borrowed from [*"Series of posts on the evolution of TransForth"*](https://blogs.msdn.microsoft.com/ashleyf/tag/transforth/). The code base is also available on [TransForth](https://github.com/AshleyF/TransForth) under MIT License.


