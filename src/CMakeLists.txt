# Source-level CMake configuration
set(CGEN_SOURCES
  generator.cppm
  main.cpp
)

# Define project binary
add_executable(cgen ${CGEN_SOURCES})

# Module compilation setup
target_sources(cgen 
  PUBLIC 
    FILE_SET CXX_MODULES FILES
      generator.cppm
)

# Link dependencies
target_link_libraries(cgen PRIVATE fmt::fmt)

# Installation
install(TARGETS cgen
  EXPORT cgen-targets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  FILE_SET CXX_MODULES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cgen
)

install(EXPORT cgen-targets
  FILE cgen-targets.cmake
  NAMESPACE cgen::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cgen
)