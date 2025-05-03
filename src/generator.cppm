module;

#include <filesystem>
#include <fmt/core.h>

export module cgen.generator;

export namespace cgen {
void hello() { fmt::print("Hello, world!\n"); }
} // namespace cgen