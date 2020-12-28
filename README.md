# fiocca
![Build Status](https://img.shields.io/travis/enzoleo/fiocca?style=for-the-badge)
![License](https://img.shields.io/github/license/enzoleo/fiocca?color=black&style=for-the-badge)
![Repo Size](https://img.shields.io/github/repo-size/enzoleo/fiocca?style=for-the-badge)

Fundamental implementations of some computational algorithms in plane geometry. It does not contain too complicated properties or theorems. However, I will upload some interesting problems (*in my own view*), that might be a little bit tedious and difficult to comprehend, to this repository :)

## Build

We enable building from `CMake`. Since this repository also tends to test some `C++20` features, make sure that your compiler has enough support. Specifically, `g++-9` or higher version is preferred. Provable compatibility is made for `C++17`, so at least work with a compiler supporting most of the  `C++17` features.

```shell
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/you/install
make && make install
```

