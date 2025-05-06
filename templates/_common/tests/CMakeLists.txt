cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(tests)

cpmaddpackage("gh:jkammerland/doctest@1.0.0")

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${TEST_SOURCES})
target_link_libraries(${PROJECT_NAME} PUBLIC doctest::doctest)

if((UNIX OR APPLE) AND NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
  target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
elseif(WIN32)
  # target_compile_options(${PROJECT_NAME} PRIVATE /fsanitize=address) target_link_options(${PROJECT_NAME} PRIVATE /fsanitize=address)
endif()
