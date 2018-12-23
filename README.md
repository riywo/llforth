# LLForth [![MIT License](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/riywo/llforth/blob/master/LICENSE.txt)
Experimental Forth implementation in LLVM.

## Getting started

### Prerequisites

#### Install
- CMake
- LLVM
- Rust

### Build
```sh
mkdir cmake-build-debug
cd cmake-build-debug
# If you install LLVM outside default path
# export LLVM_DIR=/usr/local/opt/llvm/lib/cmake
cmake ..
make llforth
```

### Enjoy!
```sh
./llforth --help
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
```sh
./llforth
> 1 1 + .
2
> : hi ." Hello world!" ;

> hi
Hello world!
```

## Supported words
https://github.com/riywo/llforth/wiki/Supported-words


## Architecture
https://github.com/riywo/llforth/wiki/Architecture


## Acknowledgments

### Forthless / "Low-Level Programming"
https://github.com/sayon/forthress


### Series of posts on the evolution of TransForth
https://blogs.msdn.microsoft.com/ashleyf/tag/transforth/


