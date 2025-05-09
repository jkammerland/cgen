cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

@USE_VCPKG@
@USE_CONAN@
@USE_CPM@

project(
  @PROJECT_NAME@
  VERSION @PROJECT_VERSION@
  DESCRIPTION "@PROJECT_DESCRIPTION@"
  LANGUAGES CXX)

set(${PROJECT_NAME}_ROOT_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
    CACHE INTERNAL "Root directory of ${PROJECT_NAME}")
set(CURRENT_ROOT_DIR ${${PROJECT_NAME}_ROOT_DIR})

message(STATUS "${PROJECT_NAME}:<${PROJECT_VERSION}>, using cmake:<${CMAKE_VERSION}>")

# Set C++ standard to C++@CPP_STANDARD@
set(CMAKE_CXX_STANDARD @CPP_STANDARD@)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find dependencies using find_package
find_package(fmt REQUIRED)

# Add subdirectories
add_subdirectory(src)

# Package configuration
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Generate and install package configuration files
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/@PROJECT_NAME@-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/@PROJECT_NAME@-config.cmake
                              INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/@PROJECT_NAME@)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/@PROJECT_NAME@-config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/@PROJECT_NAME@-config.cmake ${CMAKE_CURRENT_BINARY_DIR}/@PROJECT_NAME@-config-version.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/@PROJECT_NAME@)

# Export targets
install(
  EXPORT ${PROJECT_NAME}-targets
  FILE "${PROJECT_NAME}-targets.cmake"
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

# CPack configuration
set(CPACK_PACKAGE_NAME "@PROJECT_NAME@")
set(CPACK_PACKAGE_VENDOR "@PROJECT_VENDOR@")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "@PROJECT_DESCRIPTION@")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "@PROJECT_NAME@")
set(CPACK_PACKAGE_CONTACT "@PROJECT_CONTACT@")

include(CPack)
