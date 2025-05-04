project(from-config-generation)

cpmaddpackage("gh:marzer/tomlplusplus@3.4.0")

add_library(${PROJECT_NAME} STATIC)

# Add source files
target_sources(${PROJECT_NAME}
  PRIVATE
    toml-parser.cpp
    generator.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 23
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC
  ${CURRENT_ROOT_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE tomlplusplus::tomlplusplus fmt::fmt)