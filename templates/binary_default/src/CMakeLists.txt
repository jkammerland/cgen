# Source-level CMake configuration
add_executable(${PROJECT_NAME})

# Add source files
target_sources(${PROJECT_NAME}
  PRIVATE
    main.cpp
    # Add more source files here
)

# Add include directories
target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CURRENT_ROOT_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link dependencies
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    fmt::fmt
)

@CMAKE_OPTIONS@
@CMAKE_DEFINES@

# Installation
install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)