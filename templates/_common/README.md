# CGen - C++ Project Generator

CGen is a modern C++ project generator tool that creates C++20 module-based project structures with CMake configuration. It helps you bootstrap new C++ projects with proper namespacing, build configuration, and package management.

## Features

- Creates modern C++20/C++23 projects with module support
- Generates complete CMake configuration
- Includes package configuration for easy consumption by other projects
- Supports multiple package managers (CPM, Conan, vcpkg, xrepo)
- Customizable project structure through templates
- Configurable namespaces and dependencies
- TOML-based project configuration

## Requirements

- CMake 3.28 or higher (for C++20 modules support)
- C++23 compatible compiler
- fmt library

## Build Instructions

1. Clone the repository
2. Create a build directory and navigate to it:
   ```
   mkdir build && cd build
   ```
3. Configure with CMake:
   ```
   cmake ..
   ```
4. Build the project:
   ```
   cmake --build .
   ```
5. Optionally install:
   ```
   cmake --install .
   ```