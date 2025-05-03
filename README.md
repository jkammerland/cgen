# CGen - C++ Project Generator

CGen is a modern C++ project generator tool that creates C++20 module-based project structures with CMake configuration. It helps you bootstrap new C++ projects with proper namespacing, build configuration, and package management.

## Features

- Creates modern C++20 projects with module support
- Generates complete CMake configuration
- Includes package configuration for easy consumption by other projects
- Customizable project structure through templates
- Configurable namespaces, module names, and dependencies

## Requirements

- CMake 3.28 or higher (for C++20 modules support)
- C++20 compatible compiler
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

## Usage

```
cgen <project-name> [options]
```

### Options

- `--output <dir>`: Output directory (default: current directory)
- `--namespace <namespace>`: Project namespace (default: same as project name)
- `--module <module>`: Main module name (default: core)
- `--class <class>`: Main class name (default: Core)
- `--version <version>`: Project version (default: 0.1.0)
- `--description "<desc>"`: Project description
- `--dependency <dep>`: Add a dependency (can be used multiple times)
- `--vendor "<vendor>"`: Project vendor name
- `--contact "<email>"`: Contact email
- `--help`: Display help message

## Example

```bash
# Generate a basic project
cgen myproject

# Generate a project with custom namespace and dependencies
cgen myproject --namespace mycompany --module engine --dependency fmt --dependency spdlog
```

## Generated Project Structure

The generated project will have the following structure:

```
myproject/
├── CMakeLists.txt
├── cmake/
│   └── myproject-config.cmake.in
├── include/
│   └── myproject/
└── src/
    ├── CMakeLists.txt
    ├── core.cppm (or custom module name)
    └── main.cpp
```

## Customizing Templates

The generator uses template files from the `template/` directory. You can modify these templates to customize the generated project structure and files.

## License

[MIT License](LICENSE)