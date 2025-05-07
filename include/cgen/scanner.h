
#pragma once

#include <expected>
#include <filesystem>
#include <fmt/core.h> // For fmt::print
#include <map>        // For std::map
#include <memory>     // For std::shared_ptr, std::make_shared
#include <set>        // For std::set
#include <string>
#include <vector>

namespace cgen {
namespace fs = std::filesystem;

enum class scan_status : int {
    success = 0,
    error   = 1,
};

// Custom comparator for std::shared_ptr<Directory> based on Directory::name
template <typename T> struct CompareDirectoryByName {
    bool operator()(const std::shared_ptr<T> &lhs, const std::shared_ptr<T> &rhs) const {
        // Ensure consistent ordering if one or both pointers are null
        if (!lhs && !rhs)
            return false; // Both null, considered equal for ordering purposes
        if (!lhs)
            return true; // Null is considered "less than" non-null
        if (!rhs)
            return false; // Non-null is "greater than" null

        // Now both lhs and rhs point to Directory objects
        return lhs->name < rhs->name;
    }
};

struct Directory {
    std::string                                                             name;        // Simple name of the directory or file
    fs::path                                                                path;        // Canonical path to the directory
    std::set<std::string>                                                   files;       // Set of file names in this directory
    std::set<std::shared_ptr<Directory>, CompareDirectoryByName<Directory>> directories; // Set of subdirectories
};

/**
 * Scans a template directory and constructs a hierarchical representation of its contents.
 *
 * This function traverses the specified template directory, building a tree of `Directory` objects
 * that represent the structure of the directory. Each `Directory` contains sets of files and
 * subdirectories. Special handling is applied for top-level files, which are grouped into a
 * virtual directory named "." if no actual directory named "." exists at the root.
 *
 * @param template_name The name of the template directory to scan.
 * @param templates_base_dir The base directory path where template directories are located.
 *
 * @return A `std::expected` containing:
 *         - Success: A set of `std::shared_ptr<Directory>` objects representing the top-level
 *           directories (and virtual directory for top-level files if applicable), ordered by name.
 *         - Failure: A `scan_status` error code indicating the reason for failure.
 *
 * @note Error conditions:
 *       - If the template directory does not exist or is not a directory.
 *       - If filesystem operations (e.g., canonicalization, iteration) fail due to permission
 *         issues or other filesystem errors.
 *       - If the directory contains symbolic links that cannot be resolved.
 *
 * @note The function uses `fs::weakly_canonical` to ensure consistent path handling across
 *       different filesystem representations.
 */
std::expected<std::set<std::shared_ptr<Directory>, CompareDirectoryByName<Directory>>, scan_status>
scan_template_directory(const std::string &template_name, const std::string &templates_base_dir);

/**
 * @brief Lists template directories based on the provided configuration result.
 *
 * This function determines the location of the templates directory either from
 * the provided `result` object or defaults to `"templates/"`. It then checks if
 * the directory exists and is a valid directory. If so, it scans the directory
 * and returns a list of subdirectory names that do not start with an underscore.
 *
 * @tparam T The type of the `result` object, expected to support key-value access
 *           (e.g., a CXXParseOpts object with `count()` and `as<std::string>()` methods).
 * @param result A configuration object that may contain a `"templates"` key
 *               specifying the path to the templates directory.
 *
 * @return A `std::expected` containing a vector of template directory names on
 *         success, or a `scan_status::error` on failure.
 *
 * @throws std::exception if an error occurs while reading the directory.
 *
 * @note This function relies on the filesystem library (`<filesystem>`) and
 *       assumes the use of a CXXParseOpts-like object for `result`.
 *
 * @details
 * - If `result["templates"]` is provided and valid, that path is used.
 * - Otherwise, the default `"templates/"` is used.
 * - Only directories that are not prefixed with an underscore are included in
 *   the result.
 * - Errors such as missing directories or invalid paths are reported via
 *   `std::unexpected` and printed to stderr.
 */
template <typename T> std::expected<std::vector<std::string>, scan_status> list_templates(const T &result) {
    std::string templates_dir;

    // Determine the templates directory
    if (result["templates"].count() > 0) {
        templates_dir = result["templates"].template as<std::string>();
    } else {
        templates_dir = "templates/";
    }

    // Check if the directory exists and is a directory
    if (!fs::exists(templates_dir) || !fs::is_directory(templates_dir)) {
        fmt::print(stderr, "Error: Templates directory not found: {}\n", templates_dir);
        return std::unexpected(scan_status::error);
    }

    // Scan the templates directory
    std::vector<std::string> templates;
    try {
        for (const auto &entry : fs::directory_iterator(templates_dir)) {
            bool is_template = !entry.path().filename().string().starts_with("_");
            if (entry.is_directory() && is_template) {
                templates.push_back(entry.path().filename().string());
            }
        }
    } catch (const std::exception &e) {
        fmt::print(stderr, "Error reading templates directory: {}\n", e.what());
        return std::unexpected(scan_status::error);
    }

    return templates;
}

} // namespace cgen
