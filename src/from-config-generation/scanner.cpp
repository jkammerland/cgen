#include "cgen/scanner.h"

namespace cgen {

std::expected<std::set<std::shared_ptr<Directory>, CompareDirectoryByName<Directory>>, scan_status>
scan_template_directory(const std::string &template_name, const std::string &templates_base_dir) {

    std::set<std::shared_ptr<Directory>, CompareDirectoryByName<Directory>> top_level_dirs_result;
    fs::path template_dir_input = fs::path(templates_base_dir) / template_name;

    std::error_code ec;

    // Check if the template directory exists
    if (!fs::exists(template_dir_input, ec) || ec) {
        if (ec) {
            fmt::print(stderr, "Error checking existence of template directory {}: {}\n", template_dir_input.string(), ec.message());
        } else {
            fmt::print(stderr, "Error: Template directory not found: {}\n", template_dir_input.string());
        }
        return std::unexpected(scan_status::error);
    }

    // Check if it's a directory
    if (!fs::is_directory(template_dir_input, ec) || ec) {
        if (ec) {
            fmt::print(stderr, "Error checking if path is a directory {}: {}\n", template_dir_input.string(), ec.message());
        } else {
            fmt::print(stderr, "Error: Path is not a directory: {}\n", template_dir_input.string());
        }
        return std::unexpected(scan_status::error);
    }

    // Canonicalize the root template directory path for consistent lookups
    fs::path canonical_template_dir_root = fs::weakly_canonical(template_dir_input, ec);
    if (ec) {
        fmt::print(stderr, "Error canonicalizing template directory path {}: {}. Using non-canonical path as fallback.\n",
                   template_dir_input.string(), ec.message());
        canonical_template_dir_root = template_dir_input;
        ec.clear(); // Clear error and proceed with the potentially non-canonical path
    }

    // Maps a canonical directory path to its corresponding Directory object
    std::map<fs::path, std::shared_ptr<Directory>> created_directories_map;
    // Special Directory object for files directly under template_dir_input
    std::shared_ptr<Directory> virtual_root_dir_for_top_level_files = nullptr;

    try {
        fs::recursive_directory_iterator iter(canonical_template_dir_root, fs::directory_options::skip_permission_denied);
        fs::recursive_directory_iterator end_iter; // Default constructor creates end iterator

        for (; iter != end_iter; ++iter) {
            const auto     &entry            = *iter;
            const fs::path &raw_current_path = entry.path(); // Path from iterator, may not be canonical

            // Get canonical path for the current entry (for map keys and Directory::path)
            fs::path current_canonical_path = fs::weakly_canonical(raw_current_path, ec);
            if (ec) {
                fmt::print(stderr, "Error canonicalizing entry path {}: {}. Skipping entry.\n", raw_current_path.string(), ec.message());
                ec.clear(); // Clear error and skip this problematic entry
                continue;
            }

            // Get canonical path for the parent (for map lookups)
            fs::path parent_raw_path       = raw_current_path.parent_path();
            fs::path parent_canonical_path = fs::weakly_canonical(
                parent_raw_path, ec); // parent_raw_path could be empty if raw_current_path is a root.
                                      // For entries from recursive_directory_iterator rooted at canonical_template_dir_root,
                                      // parent_path should be valid or equal to canonical_template_dir_root.
            if (ec) {
                fmt::print(stderr, "Error canonicalizing parent path of {}: {}. Skipping entry.\n", raw_current_path.string(),
                           ec.message());
                ec.clear(); // Clear error and skip this problematic entry
                continue;
            }

            bool is_file = entry.is_regular_file(ec); // is_regular_file follows symlinks
            if (ec) {
                fmt::print(stderr, "Error checking type of {}: {}. Skipping.\n", raw_current_path.string(), ec.message());
                ec.clear(); // Clear error and skip
                continue;
            }

            if (is_file) {
                std::string filename = raw_current_path.filename().string(); // Use filename from raw path for Directory::name

                if (parent_canonical_path == canonical_template_dir_root) {
                    // File is directly under the root template directory
                    if (!virtual_root_dir_for_top_level_files) {
                        virtual_root_dir_for_top_level_files       = std::make_shared<Directory>();
                        virtual_root_dir_for_top_level_files->name = ".";                         // Virtual directory name
                        virtual_root_dir_for_top_level_files->path = canonical_template_dir_root; // Path it represents
                    }
                    virtual_root_dir_for_top_level_files->files.insert(filename);
                } else {
                    // File is in a subdirectory
                    auto map_it = created_directories_map.find(parent_canonical_path);
                    if (map_it != created_directories_map.end()) {
                        map_it->second->files.insert(filename);
                    } else {
                        fmt::print(stderr, "Warning: Parent directory (canonical: {}) for file {} not found in map. File skipped.\n",
                                   parent_canonical_path.string(), raw_current_path.string());
                    }
                }
            } else { // Not a regular file, check if it's a directory
                bool is_subdir =
                    entry.is_directory(ec); // is_directory follows symlinks.
                                            // recursive_directory_iterator itself does not follow directory symlinks by default.
                if (ec) {
                    fmt::print(stderr, "Error checking type of {}: {}. Skipping.\n", raw_current_path.string(), ec.message());
                    ec.clear(); // Clear error and skip
                    continue;
                }

                if (is_subdir) {
                    auto new_dir_node  = std::make_shared<Directory>();
                    new_dir_node->name = raw_current_path.filename().string(); // Name from original filename
                    new_dir_node->path = current_canonical_path;               // Store the canonical path of the subdir

                    // Store this new directory node in the map using its canonical path as the key
                    created_directories_map[current_canonical_path] = new_dir_node;

                    if (parent_canonical_path == canonical_template_dir_root) {
                        // This is a direct subdirectory of the root template directory
                        top_level_dirs_result.insert(new_dir_node);
                    } else {
                        // This is a nested directory, add it to its parent's list of directories
                        auto map_it = created_directories_map.find(parent_canonical_path);
                        if (map_it != created_directories_map.end()) {
                            map_it->second->directories.insert(new_dir_node);
                        } else {
                            // This can happen if parent was skipped due to permissions or other errors
                            fmt::print(stderr,
                                       "Warning: Parent directory (canonical: {}) for subdirectory {} not found in map. Subdirectory not "
                                       "fully linked.\n",
                                       parent_canonical_path.string(), raw_current_path.string());
                        }
                    }
                }
                // Other types (symlinks not pointing to regular file/dir, block devices, sockets, etc.) are ignored.
            }
        }
    } catch (const fs::filesystem_error &e) {
        fmt::print(stderr, "Filesystem error during scan of {}: {}\n", canonical_template_dir_root.string(), e.what());
        return std::unexpected(scan_status::error);
    } catch (const std::exception &e) { // Catch other potential errors like std::bad_alloc
        fmt::print(stderr, "General error during scan of {}: {}\n", canonical_template_dir_root.string(), e.what());
        return std::unexpected(scan_status::error);
    }

    // After iterating, if the virtual directory for top-level files was created and has files, add it to results.
    if (virtual_root_dir_for_top_level_files && !virtual_root_dir_for_top_level_files->files.empty()) {
        bool conflicting_dot_dir_exists = false;
        // Check if an *actual* directory named "." was already found at the top level
        for (const auto &dir_ptr : top_level_dirs_result) {
            if (dir_ptr && dir_ptr->name == ".") {
                // An actual directory from the filesystem named "." exists
                conflicting_dot_dir_exists = true;
                break;
            }
        }
        if (!conflicting_dot_dir_exists) {
            top_level_dirs_result.insert(virtual_root_dir_for_top_level_files);
        }
    }

    return top_level_dirs_result;
}
} // namespace cgen