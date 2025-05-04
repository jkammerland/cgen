add_subdirectory(from-config-generation)

# Source-level CMake configuration
add_executable(${PROJECT_NAME})

# Add source files
target_sources(${PROJECT_NAME} PRIVATE 
  main.cpp
)

# Link dependencies
cpmaddpackage("gh:jarro2783/cxxopts@3.2.0")
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt from-config-generation cxxopts::cxxopts)

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC
  ${CURRENT_ROOT_DIR}/include
)

# Just install the executable without export
install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
