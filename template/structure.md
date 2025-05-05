# Template Structure

This document outlines the organization of the template files used by CGen.

## Directory Structure

```
template/
│
├── core/                          # Core project structure templates
│   ├── root_CMakeLists.txt        # Root CMakeLists.txt template
│   ├── src_CMakeLists.txt         # Source directory CMakeLists.txt 
│   ├── config.cmake.in            # CMake package configuration template
│   └── find_packages.cmake        # Find package dependencies template
│
├── project_types/                 # Project-type specific templates
│   ├── binary/                    # Binary/executable project templates
│   │   ├── CMakeLists.txt         # Binary-specific CMakeLists.txt
│   │   └── main.cpp               # Main entry point for binary projects
│   │
│   └── library/                   # Library project templates
│       ├── CMakeLists.txt         # Library-specific CMakeLists.txt
│       └── module.cppm            # C++20 module template for libraries
│
├── package_managers/              # Package manager configuration templates
│   ├── cpm/                       # CPM (CMake Package Manager) templates
│   │   ├── dependencies.cmake     # Main CPM dependencies file
│   │   └── dependency.cmake       # Individual CPM dependency template
│   │
│   ├── conan/                     # Conan package manager templates
│   │   └── conanfile.txt          # Conan configuration file
│   │
│   ├── vcpkg/                     # Vcpkg package manager templates
│   │   └── vcpkg.json             # Vcpkg manifest file
│   │
│   └── xrepo/                     # XRepo package manager templates
│       └── xmake.lua              # XMake configuration file
│
└── README.md                      # Documentation about templates
```

## Placeholder Format

All templates use a consistent placeholder format with curly braces: `{PLACEHOLDER_NAME}`.

## Template Categories

1. **Core Templates**
   - Essential templates for any C++ project
   - Used regardless of project type or package manager

2. **Project Type Templates**
   - Binary/executable specific templates
   - Library specific templates
   - Header-only library specific templates

3. **Package Manager Templates**
   - CPM (CMake Package Manager) 
   - Conan
   - Vcpkg
   - XRepo

## Common Placeholders

- `{PROJECT_NAME}` - Project name
- `{PROJECT_VERSION}` - Project version
- `{PROJECT_DESCRIPTION}` - Project description
- `{PROJECT_NAMESPACE}` - Project namespace
- `{CPP_STANDARD}` - C++ standard (e.g., 20, 23)
- `{DEPENDENCIES}` - Project dependencies
- `{CMAKE_OPTIONS}` - CMake options
- `{CMAKE_DEFINES}` - CMake defines