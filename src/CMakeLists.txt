add_subdirectory(parser)

# Source-level CMake configuration
add_executable(${PROJECT_NAME})

# Add main.cpp as regular source file
target_sources(${PROJECT_NAME} PRIVATE main.cpp)

# Add modules with proper configuration
target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET CXX_MODULES 
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    FILES generator.cppm
)

# Link dependencies
cpmaddpackage("gh:jarro2783/cxxopts@3.2.0")
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt config-parser cxxopts::cxxopts)

# Just install the executable without export
install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
