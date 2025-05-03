project(config-parser)

cpmaddpackage("gh:marzer/tomlplusplus@3.4.0")

add_library(${PROJECT_NAME} STATIC)
target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET CXX_MODULES 
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    FILES toml-parser.cppm
)
set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 23
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)
target_link_libraries(${PROJECT_NAME} PRIVATE tomlplusplus::tomlplusplus fmt::fmt)