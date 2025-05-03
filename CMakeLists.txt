cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

project(cgen
  VERSION 0.1.0
  DESCRIPTION "C++ Project Generator"
  LANGUAGES CXX
)

message(STATUS "${PROJECT_NAME}:<${PROJECT_VERSION}>, using cmake:<${CMAKE_VERSION}>")

# Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/get_cpm.cmake)
get_cpm("0.40.8" "78ba32abdf798bc616bab7c73aac32a17bbd7b06ad9e26a6add69de8f3ae4791")

cpmaddpackage("gh:fmtlib/fmt#11.1.4")

# Find dependencies
find_package(fmt CONFIG REQUIRED)

# Add subdirectories
add_subdirectory(src)

# Package configuration
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Installation targets
install(DIRECTORY include/cgen
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

# Generate and install package configuration files
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cgen-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cgen-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cgen
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/cgen-config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/cgen-config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cgen-config-version.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cgen
)

# CPack configuration
set(CPACK_PACKAGE_NAME "cgen")
set(CPACK_PACKAGE_VENDOR "Your Organization")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++ Project Generator")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "cgen")
set(CPACK_PACKAGE_CONTACT "Your Email")

include(CPack)