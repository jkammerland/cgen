# Source-level CMake configuration
add_library(${PROJECT_NAME})

# Add source files
target_sources(${PROJECT_NAME}
  PRIVATE
@SOURCE_FILES@
@MODULE_FILES@
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
  EXPORT ${PROJECT_NAME}-targets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)