#include "cgen/scanner.h"

#include <algorithm>
#include <cgen/placeholder_processor.h>
#include <cxxopts.hpp>
#include <expected>
#include <filesystem>
#include <fmt/core.h>
#include <fstream>    // Added for file operations
#include <functional> // Added for std::function
#include <sstream>    // Added for std::stringstream
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace cgen;

int main(int argc, char *argv[]) {
    try {
        // Parse the command line arguments
        cxxopts::Options options("cgen", "C++ Project Generator");
        options.add_options()("h,help", "Print help")("l,list", "List available templates", cxxopts::value<bool>()->default_value("false"))(
            "g,generate", "Generate project from template", cxxopts::value<std::string>()) // Added --generate
            ("o,output", "Output directory", cxxopts::value<std::string>()->default_value("."))(
                "gui", "Run the terminal user interface",
                cxxopts::value<bool>()->default_value("false"))("templates", "Custom templates directory", cxxopts::value<std::string>());

        auto result = options.parse(argc, argv);

        if (result["help"].as<bool>() || (result.arguments().empty() && !result.count("generate"))) {
            fmt::print("{}\n", options.help());
            return 0;
        }
        if (result["list"].as<bool>()) {
            auto result_or = list_templates(result);

            if (result_or) {
                fmt::print("Available templates:\n");
                for (const auto &name : result_or.value()) {
                    fmt::print("  {}\n", name);
                }
                return 0;

            } else {
                return static_cast<int>(result_or.error());
            }
        }
        if (result["gui"].as<bool>()) {
            // Run the terminal user interface
            throw std::runtime_error("Not implemented yet for gui"); // Updated message
        }

        if (result.count("generate")) {
            std::string template_name = result["generate"].as<std::string>();
            fs::path    output_dir    = result["output"].as<std::string>();
            std::string templates_base_dir_str;

            if (result.count("templates")) {
                templates_base_dir_str = result["templates"].as<std::string>();
            } else {
                templates_base_dir_str = "templates/";
            }

            fmt::print("Generating project from template '{}' into directory '{}' using base '{}'\n", template_name, output_dir.string(),
                       templates_base_dir_str);

            // 1. Validate template existence
            auto available_templates_or = list_templates(result);
            if (!available_templates_or) {
                fmt::print(stderr, "Error: Could not list available templates to validate.\n");
                return static_cast<int>(available_templates_or.error());
            }
            const auto &available_templates = available_templates_or.value();
            if (std::find(available_templates.begin(), available_templates.end(), template_name) == available_templates.end()) {
                fmt::print(stderr, "Error: Template '{}' not found in {}.\nAvailable templates:\n", template_name, templates_base_dir_str);
                for (const auto &name : available_templates) {
                    fmt::print(stderr, "  {}\n", name);
                }
                return 1;
            }

            // 2. Scan the template directory
            auto scanned_template_or = scan_template_directory(template_name, templates_base_dir_str);
            if (!scanned_template_or) {
                fmt::print(stderr, "Error scanning template directory '{}'.\n", template_name);
                return static_cast<int>(scanned_template_or.error());
            }
            const auto &top_level_entries = scanned_template_or.value();

            // 3. Prepare for placeholder processing
            PlaceholderProcessor processor; // Uses default style: @PLACEHOLDER@
            // TODO: Dynamically collect placeholder values (e.g., from user input or a config file)
            std::unordered_map<std::string, std::string> placeholder_values = {{"PROJECT_NAME", "MyGeneratedProject"},
                                                                               {"AUTHOR_NAME", "CGen User"},
                                                                               {"APP_NAME", "DefaultApp"}};

            fs::path output_base_path = fs::absolute(output_dir);
            if (!fs::exists(output_base_path)) {
                if (!fs::create_directories(output_base_path)) {
                    fmt::print(stderr, "Error: Could not create output directory: {}\n", output_base_path.string());
                    return 1;
                }
            } else if (!fs::is_directory(output_base_path)) {
                fmt::print(stderr, "Error: Output path exists but is not a directory: {}\n", output_base_path.string());
                return 1;
            }

            // 4. Recursive function to process directory entries
            std::function<void(const std::shared_ptr<Directory> &, const fs::path &)> process_entry_recursively;
            process_entry_recursively = [&](const std::shared_ptr<Directory> &dir_entry, const fs::path &current_output_dir_path) {
                fs::path next_output_target_path;
                // The virtual "." directory means files/dirs are at the current level
                if (dir_entry->name == ".") {
                    next_output_target_path = current_output_dir_path;
                } else {
                    next_output_target_path = current_output_dir_path / dir_entry->name;
                    if (!fs::exists(next_output_target_path)) {
                        if (!fs::create_directories(next_output_target_path)) {
                            fmt::print(stderr, "Error: Could not create directory: {}\n", next_output_target_path.string());
                            // Potentially throw or return an error code here to stop generation
                            return;
                        }
                        fmt::print("Created directory: {}\n", next_output_target_path.string());
                    }
                }

                // Process files in the current directory entry
                for (const auto &file_name : dir_entry->files) {
                    // dir_entry->path is the canonical path to the source directory of this entry
                    fs::path source_file_path = dir_entry->path / file_name;
                    fs::path dest_file_path   = next_output_target_path / file_name;

                    // TODO: Implement filename templating if needed (e.g., @PROJECT_NAME@.cpp)

                    try {
                        std::ifstream tpl_file_stream(source_file_path);
                        if (!tpl_file_stream) {
                            fmt::print(stderr, "Warning: Could not open template file for reading: {}\n", source_file_path.string());
                            continue;
                        }
                        std::stringstream buffer;
                        buffer << tpl_file_stream.rdbuf();
                        tpl_file_stream.close();
                        std::string content = buffer.str();

                        std::string processed_content = processor.replacePlaceholders(content, placeholder_values);

                        std::ofstream out_file_stream(dest_file_path);
                        if (!out_file_stream) {
                            fmt::print(stderr, "Error: Could not open output file for writing: {}\n", dest_file_path.string());
                            continue;
                        }
                        out_file_stream << processed_content;
                        out_file_stream.close();
                        fmt::print("Generated file: {}\n", dest_file_path.string());

                    } catch (const std::exception &e) {
                        fmt::print(stderr, "Error processing file {} to {}: {}\n", source_file_path.string(), dest_file_path.string(),
                                   e.what());
                    }
                }

                // Recursively process subdirectories
                for (const auto &sub_dir_entry : dir_entry->directories) {
                    // Pass the path where this subdirectory's contents should go
                    process_entry_recursively(sub_dir_entry, next_output_target_path);
                }
            };

            // 5. Start processing from top-level entries
            for (const auto &top_level_dir_entry : top_level_entries) {
                process_entry_recursively(top_level_dir_entry, output_base_path);
            }

            fmt::print("Project generation complete for template '{}' in '{}'.\n", template_name, output_base_path.string());
            return 0;
        }

    } catch (const cxxopts::exceptions::exception &e) { // More specific catch for cxxopts
        fmt::print(stderr, "Error parsing options: {}\n", e.what());
        return 1;
    } catch (const std::exception &e) {
        fmt::print(stderr, "Error: {}\n", e.what());
        return 1;
    }

    // If no command was explicitly handled (e.g. only -o or --templates was given without a command)
    // This part might be reached if only options like -o are passed without a primary command like --list or --generate
    // Depending on desired behavior, you might want to print help here.
    // For now, if no command like --generate, --list, --gui, or --help is hit, and arguments are not empty,
    // it implies an invalid combination or missing command.
    // The check `result.arguments().empty()` at the top already handles the no-argument case.
    // If options are present but no command, cxxopts might throw or one of the if conditions would be met.
    // If we reach here, it might be an unhandled scenario or a logic gap.
    // A simple fallback could be to print help.
    // For now, we assume valid commands lead to an exit within their blocks.
    // If generate is not specified and other conditions are not met, it might fall through.
    // Adding a fallback:
    fmt::print(stderr, "No valid command specified. Use --help for options.\n");
    return 1;
}