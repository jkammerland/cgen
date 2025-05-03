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

## Usage

```
cgen -i <config-file.toml> -o <output-dir>
```

### Options

- `-i, --input <file>`: Input TOML configuration file
- `-o, --output <dir>`: Output directory (default: current directory)
- `-t, --tui`: Run in terminal user interface mode
- `-h, --help`: Display help message

## Using TOML Configuration

CGen uses TOML files for project configuration. This allows for more fine-grained control over the generated project.

### Example TOML Configuration

```toml
# Basic project information
[project]
name = "my_project"              # Project name (required)
version = "0.1.0"                # Project version
description = "A simple project" # Project description
namespace = "myproject"          # Namespace for the code (default: project name)
vendor = "Your Organization"     # Vendor/organization name
contact = "your.email@example.com" # Contact email

# Project type configuration
[project.type]
type = "binary"                  # Project type: "binary", "library", "header_only"

# Dependencies
[dependencies]
fmt = { version = "9.1.0", required = true }
spdlog = { version = "1.11.0", required = false }

# Package managers
[package_managers]
cpm = true                       # Use CPM.cmake
conan = false                    # Use Conan
vcpkg = false                    # Use vcpkg
xrepo = false                    # Use xrepo

# Build configuration
[build]
cpp_standard = "23"              # C++ standard: "20", "23"
enable_testing = true            # Enable testing
use_modules = true               # Use C++20 modules

# CMake options
[build.cmake_options]
BUILD_SHARED_LIBS = true         # Build shared libraries

# CMake defines
[build.cmake_defines]
VERSION_INFO = "\"${PROJECT_VERSION}\""  # Version info define

# Templates to include
[templates]
main = true                      # Include main.cpp
cmake_root = true                # Include root CMakeLists.txt
cmake_src = true                 # Include src CMakeLists.txt
cmake_config = true              # Include cmake config

# Package manager templates
[templates.package_managers]
conan_config = false             # Include conan configuration
vcpkg_config = false             # Include vcpkg configuration
xrepo_config = false             # Include xrepo configuration
```

## TOML Schema

The TOML configuration schema supports the following sections:

### Project Section

Basic project information:

```toml
[project]
name = "my_project"              # Project name (required)
version = "0.1.0"                # Project version
description = "A simple project" # Project description
namespace = "myproject"          # Namespace for the code
vendor = "Your Organization"     # Vendor/organization name
contact = "your.email@example.com" # Contact email

[project.type]
type = "binary"                  # Project type: "binary", "library", "header_only"
```

### Dependencies

Define project dependencies:

```toml
[dependencies]
# Format: package_name = { version = "x.y.z", required = true }
fmt = { version = "9.1.0", required = true }
spdlog = { version = "1.11.0", required = false }
```

### Package Managers

Configure package managers:

```toml
[package_managers]
cpm = true                       # Use CPM.cmake
conan = false                    # Use Conan
vcpkg = false                    # Use vcpkg
xrepo = false                    # Use xrepo
```

### Build Configuration

Configure build settings:

```toml
[build]
cpp_standard = "23"              # C++ standard: "20", "23"
enable_testing = true            # Enable testing
use_modules = true               # Use C++20 modules

# CMake options
[build.cmake_options]
BUILD_SHARED_LIBS = true         # Build shared libraries
CMAKE_POSITION_INDEPENDENT_CODE = true # Position independent code

# CMake defines
[build.cmake_defines]
VERSION_INFO = "\"${PROJECT_VERSION}\""  # Version info define
DEBUG_MODE = ""                  # Empty value define
```

### Templates

Configure which templates to include:

```toml
[templates]
main = true                      # Include main.cpp
cmake_root = true                # Include root CMakeLists.txt
cmake_src = true                 # Include src CMakeLists.txt
cmake_config = true              # Include cmake config

# Package manager templates
[templates.package_managers]
conan_config = false             # Include conan configuration
vcpkg_config = false             # Include vcpkg configuration
xrepo_config = false             # Include xrepo configuration

# Custom templates (optional)
[templates.custom]
# Format: template_name = { source = "path/to/template", destination = "path/in/project" }
readme = { source = "custom/readme.md.template", destination = "README.md" }
```

## Example

```bash
# Generate a project using a TOML configuration file
cgen -i examples/simple_binary.toml -o my_project
```

## Generated Project Structure

The generated project will have the following structure:

```
my_project/
├── CMakeLists.txt
├── cmake/
│   └── my_project-config.cmake.in
├── include/
│   └── my_project/
└── src/
    ├── CMakeLists.txt
    └── main.cpp
```

For library projects, the structure is similar, but may include module files instead of a main.cpp file.

## Customizing Templates

The generator uses template files from the `template/` directory. You can modify these templates to customize the generated project structure and files.

## License

[MIT License](LICENSE)