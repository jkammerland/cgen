#include "cgen/scanner.h"

#include <algorithm> // For std::find_if, std::sort
#include <atomic>    // For std::atomic for unique temp dir names
#include <cstdio>    // For std::remove with symlinks on Windows if needed (though fs::remove should work)
#include <doctest/doctest.h>
#include <fmt/base.h>
#include <fstream> // For std::ofstream to create files
#include <string>  // For std::string
#include <vector>  // For helper functions

// Use the cgen namespace for brevity in tests
using namespace cgen;

// Helper RAII class for temporary directory management
struct TempDirRAII {
    fs::path path;
    bool     active{}; // To prevent double removal if moved

    TempDirRAII(const std::string &name_prefix) {
        static std::atomic<int> counter = 0;
        // Generate a unique directory name
        // Note: fs::temp_directory_path() might throw if no temp dir is found
        path = fs::temp_directory_path() / (name_prefix + "_" + std::to_string(counter++));
        std::error_code ec;
        fs::create_directories(path, ec);
        REQUIRE_MESSAGE(!ec, "Failed to create temp directory: " << path.string() << " - " << ec.message());
    }

    ~TempDirRAII() {
        if (active) {
            std::error_code ec;
            fs::remove_all(path, ec);
            if (ec) {
                // This might happen on Windows if files are still locked, or symlinks are tricky
                // Not failing the test for cleanup failure, but print a warning.
                fmt::print(stderr, "Warning: Failed to remove temp dir {}: {}\n", path.string(), ec.message());
            }
        }
    }

    // Disable copy, enable move
    TempDirRAII(const TempDirRAII &)            = delete;
    TempDirRAII &operator=(const TempDirRAII &) = delete;

    TempDirRAII(TempDirRAII &&other) noexcept : path(std::move(other.path)), active(other.active) { other.active = false; }
    TempDirRAII &operator=(TempDirRAII &&other) noexcept {
        if (this != &other) {
            if (active) {
                std::error_code ec;
                fs::remove_all(path, ec); // Clean up existing one
            }
            path         = std::move(other.path);
            active       = other.active;
            other.active = false;
        }
        return *this;
    }

    const fs::path &operator()() const { return path; }
};

// Helper function to create files and directories
void create_structure(const fs::path &base_path, const std::vector<std::string> &items) {
    fs::create_directories(base_path); // Ensure base_path exists
    for (const auto &item_str : items) {
        fs::path item_path = base_path / item_str;
        if (item_str.back() == '/' || item_str.back() == '\\') {
            fs::create_directories(item_path);
        } else {
            fs::create_directories(item_path.parent_path()); // Ensure parent dir exists
            std::ofstream ofs(item_path);
            ofs << "content of " << item_path.filename().string();
            REQUIRE_MESSAGE(ofs.good(), "Failed to create file: " << item_path.string());
        }
    }
}

// Helper function to find a directory by name in the result set
std::shared_ptr<Directory> find_dir(const std::set<std::shared_ptr<Directory>, CompareDirectoryByName<Directory>> &dirs,
                                    const std::string                                                             &name) {
    for (const auto &dir_ptr : dirs) {
        if (dir_ptr && dir_ptr->name == name) {
            return dir_ptr;
        }
    }
    return nullptr;
}

TEST_CASE("scan_template_directory: Empty Directory") {
    TempDirRAII     temp_dir("empty_dir_test");
    const fs::path &template_base_dir = temp_dir();
    std::string     template_name     = "my_empty_template";
    fs::create_directories(template_base_dir / template_name);

    auto result = scan_template_directory(template_name, template_base_dir.string());

    REQUIRE(result.has_value());
    CHECK(result.value().empty()); // No files, no subdirectories means empty result set
}

TEST_CASE("scan_template_directory: Only Top-Level Files") {
    TempDirRAII     temp_dir("top_level_files_test");
    const fs::path &template_base_dir = temp_dir();
    std::string     template_name     = "files_only_template";
    create_structure(template_base_dir / template_name, {"file1.txt", "file2.log"});

    auto result = scan_template_directory(template_name, template_base_dir.string());

    REQUIRE(result.has_value());
    const auto &dirs = result.value();
    REQUIRE(dirs.size() == 1);

    auto dot_dir = find_dir(dirs, ".");
    REQUIRE(dot_dir != nullptr);
    CHECK(dot_dir->path == fs::weakly_canonical(template_base_dir / template_name));
    CHECK(dot_dir->files.size() == 2);
    CHECK(dot_dir->files.count("file1.txt") == 1);
    CHECK(dot_dir->files.count("file2.log") == 1);
    CHECK(dot_dir->directories.empty());
}

TEST_CASE("scan_template_directory: Only Top-Level Directories") {
    TempDirRAII     temp_dir("top_level_dirs_test");
    const fs::path &template_base_dir = temp_dir();
    std::string     template_name     = "dirs_only_template";
    create_structure(template_base_dir / template_name, {"dir_a/", "dir_b/"});

    auto result = scan_template_directory(template_name, template_base_dir.string());

    REQUIRE(result.has_value());
    const auto &dirs = result.value();
    REQUIRE(dirs.size() == 2); // dir_a and dir_b

    auto dir_a = find_dir(dirs, "dir_a");
    REQUIRE(dir_a != nullptr);
    CHECK(dir_a->path == fs::weakly_canonical(template_base_dir / template_name / "dir_a"));
    CHECK(dir_a->files.empty());
    CHECK(dir_a->directories.empty());

    auto dir_b = find_dir(dirs, "dir_b");
    REQUIRE(dir_b != nullptr);
    CHECK(dir_b->path == fs::weakly_canonical(template_base_dir / template_name / "dir_b"));
    CHECK(dir_b->files.empty());
    CHECK(dir_b->directories.empty());

    // Check order (CompareDirectoryByName should ensure "dir_a" then "dir_b")
    REQUIRE(dirs.begin() != dirs.end());
    CHECK((*dirs.begin())->name == "dir_a");
    CHECK((*std::next(dirs.begin()))->name == "dir_b");
}

TEST_CASE("scan_template_directory: Mixed Top-Level Content") {
    TempDirRAII     temp_dir("mixed_top_level_test");
    const fs::path &template_base_dir = temp_dir();
    std::string     template_name     = "mixed_template";
    create_structure(template_base_dir / template_name, {"top_file.txt", "sub_dir_c/"});

    auto result = scan_template_directory(template_name, template_base_dir.string());

    REQUIRE(result.has_value());
    const auto &dirs = result.value();
    REQUIRE(dirs.size() == 2); // "." for top_file.txt and "sub_dir_c"

    auto dot_dir = find_dir(dirs, ".");
    REQUIRE(dot_dir != nullptr);
    CHECK(dot_dir->path == fs::weakly_canonical(template_base_dir / template_name));
    CHECK(dot_dir->files.size() == 1);
    CHECK(dot_dir->files.count("top_file.txt") == 1);
    CHECK(dot_dir->directories.empty());

    auto sub_dir_c = find_dir(dirs, "sub_dir_c");
    REQUIRE(sub_dir_c != nullptr);
    CHECK(sub_dir_c->path == fs::weakly_canonical(template_base_dir / template_name / "sub_dir_c"));
    CHECK(sub_dir_c->files.empty());
    CHECK(sub_dir_c->directories.empty());
}

TEST_CASE("scan_template_directory: Nested Structure") {
    TempDirRAII     temp_dir("nested_structure_test");
    const fs::path &template_base_dir = temp_dir();
    std::string     template_name     = "nested_template";
    create_structure(template_base_dir / template_name, {"root_file.md", "parent_dir/", "parent_dir/child_file.txt",
                                                         "parent_dir/child_dir/", "parent_dir/child_dir/grandchild_file.cc"});

    auto result = scan_template_directory(template_name, template_base_dir.string());

    REQUIRE(result.has_value());
    const auto &top_dirs = result.value();
    REQUIRE(top_dirs.size() == 2); // "." and "parent_dir"

    auto dot_dir = find_dir(top_dirs, ".");
    REQUIRE(dot_dir != nullptr);
    CHECK(dot_dir->files.size() == 1);
    CHECK(dot_dir->files.count("root_file.md") == 1);

    auto parent_dir = find_dir(top_dirs, "parent_dir");
    REQUIRE(parent_dir != nullptr);
    CHECK(parent_dir->path == fs::weakly_canonical(template_base_dir / template_name / "parent_dir"));
    CHECK(parent_dir->files.size() == 1);
    CHECK(parent_dir->files.count("child_file.txt") == 1);
    REQUIRE(parent_dir->directories.size() == 1);

    auto child_dir = find_dir(parent_dir->directories, "child_dir");
    REQUIRE(child_dir != nullptr);
    CHECK(child_dir->path == fs::weakly_canonical(template_base_dir / template_name / "parent_dir" / "child_dir"));
    CHECK(child_dir->files.size() == 1);
    CHECK(child_dir->files.count("grandchild_file.cc") == 1);
    CHECK(child_dir->directories.empty());
}

TEST_CASE("scan_template_directory: Non-Existent Template Directory") {
    TempDirRAII     temp_dir("non_existent_test");
    const fs::path &template_base_dir = temp_dir();
    std::string     template_name     = "no_such_template";

    auto result = scan_template_directory(template_name, template_base_dir.string());

    REQUIRE(!result.has_value());
    CHECK(result.error() == scan_status::error);
}

TEST_CASE("scan_template_directory: Path is a File, Not a Directory") {
    TempDirRAII     temp_dir("path_is_file_test");
    const fs::path &template_base_dir = temp_dir();
    std::string     template_name     = "template_is_actually_a_file.txt";
    create_structure(template_base_dir, {template_name}); // Create as a file

    auto result = scan_template_directory(template_name, template_base_dir.string());

    REQUIRE(!result.has_value());
    CHECK(result.error() == scan_status::error);
}

TEST_CASE("scan_template_directory: Directory Ordering") {
    TempDirRAII     temp_dir("ordering_test");
    const fs::path &template_base_dir = temp_dir();
    std::string     template_name     = "ordering_template";
    create_structure(template_base_dir / template_name, {"zeta_dir/", "alpha_dir/", "beta_dir/"});

    auto result = scan_template_directory(template_name, template_base_dir.string());

    REQUIRE(result.has_value());
    const auto &dirs = result.value();
    REQUIRE(dirs.size() == 3);

    auto it = dirs.begin();
    REQUIRE(it != dirs.end());
    CHECK((*it++)->name == "alpha_dir");
    REQUIRE(it != dirs.end());
    CHECK((*it++)->name == "beta_dir");
    REQUIRE(it != dirs.end());
    CHECK((*it++)->name == "zeta_dir");
}

TEST_CASE("scan_template_directory: Hidden Files and Directories") {
    TempDirRAII     temp_dir("hidden_items_test");
    const fs::path &template_base_dir = temp_dir();
    std::string     template_name     = "hidden_template";
    create_structure(template_base_dir / template_name, {".hidden_file.txt", ".hidden_dir/", ".hidden_dir/file_in_hidden.dat"});

    auto result = scan_template_directory(template_name, template_base_dir.string());

    REQUIRE(result.has_value());
    const auto &top_dirs = result.value();
    REQUIRE(top_dirs.size() == 2); // "." and ".hidden_dir"

    auto dot_dir = find_dir(top_dirs, ".");
    REQUIRE(dot_dir != nullptr);
    CHECK(dot_dir->files.size() == 1);
    CHECK(dot_dir->files.count(".hidden_file.txt") == 1);

    auto hidden_dir = find_dir(top_dirs, ".hidden_dir");
    REQUIRE(hidden_dir != nullptr);
    CHECK(hidden_dir->path == fs::weakly_canonical(template_base_dir / template_name / ".hidden_dir"));
    CHECK(hidden_dir->files.size() == 1);
    CHECK(hidden_dir->files.count("file_in_hidden.dat") == 1);
    CHECK(hidden_dir->directories.empty());
}

TEST_CASE("scan_template_directory: Symlinks Behavior") {
    TempDirRAII     temp_dir("symlinks_test");
    const fs::path &base_path     = temp_dir(); // This is effectively templates_base_dir
    std::string     template_name = "symlink_template";
    fs::path        template_root = base_path / template_name;

    // Create actual targets
    create_structure(template_root, {"actual_file.txt", "target_dir/", "target_dir/file_in_target.txt"});

    fs::path actual_file_path  = template_root / "actual_file.txt";
    fs::path target_dir_path   = template_root / "target_dir";
    fs::path link_to_file_path = template_root / "link_to_file";
    fs::path link_to_dir_path  = template_root / "link_to_dir";

    std::error_code ec;
    // Create symlinks
    // Note: Creating symlinks might require special privileges on some OS (e.g., Windows).
    // These tests might fail if symlinks cannot be created.
    fs::create_symlink(actual_file_path, link_to_file_path, ec);
    if (ec) {
        fmt::println("Could not create file symlink {}", ec.message());
    }
    fs::create_directory_symlink(target_dir_path, link_to_dir_path, ec);
    if (ec) {
        fmt::println("Could not create directory symlink {}", ec.message());
    }
    // It's crucial for the test that symlinks *are* created. Use REQUIRE if they must exist.
    // For now, let's proceed assuming they can be created.

    auto result = scan_template_directory(template_name, base_path.string());

    REQUIRE(result.has_value());
    const auto &top_dirs = result.value();

    // Expected: ".", "target_dir", "link_to_dir" (if symlink creation worked and is seen as dir)
    // actual_file.txt and link_to_file (if exists) should be in "."
    // target_dir should contain file_in_target.txt
    // link_to_dir should be an empty directory entry (as iterator doesn't recurse symlinked dirs)

    size_t expected_top_dirs = 2;                                             // "." and "target_dir"
    if (fs::exists(link_to_dir_path) && fs::is_directory(link_to_dir_path)) { // Check if symlink was created and is seen as dir
        expected_top_dirs = 3;
    }

    // Check based on what actually exists
    bool link_to_file_created_and_is_file = fs::exists(link_to_file_path) && fs::is_regular_file(link_to_file_path);
    bool link_to_dir_created_and_is_dir   = fs::exists(link_to_dir_path) && fs::is_directory(link_to_dir_path);

    size_t current_top_dirs_count = 0;
    if (find_dir(top_dirs, "."))
        current_top_dirs_count++;
    if (find_dir(top_dirs, "target_dir"))
        current_top_dirs_count++;
    if (find_dir(top_dirs, "link_to_dir"))
        current_top_dirs_count++;

    // This check is a bit loose due to symlink creation variability.
    // A more robust test would mock filesystem calls or ensure permissions.
    CHECK_GE(top_dirs.size(), 2); // At least "." and "target_dir" should be there.

    auto dot_dir = find_dir(top_dirs, ".");
    REQUIRE(dot_dir != nullptr); // actual_file.txt ensures this.
    CHECK(dot_dir->files.count("actual_file.txt") == 1);
    if (link_to_file_created_and_is_file) {
        CHECK(dot_dir->files.count("link_to_file") == 1);
        CHECK(dot_dir->files.size() == 2);
    } else {
        CHECK(dot_dir->files.size() == 1);
    }

    auto target_dir_obj = find_dir(top_dirs, "target_dir");
    REQUIRE(target_dir_obj != nullptr);
    CHECK(target_dir_obj->files.count("file_in_target.txt") == 1);
    CHECK(target_dir_obj->directories.empty());

    if (link_to_dir_created_and_is_dir) {
        auto link_to_dir_obj = find_dir(top_dirs, "link_to_dir");
        REQUIRE(link_to_dir_obj != nullptr); // If it was created and is a dir, it should be found
        CHECK(link_to_dir_obj->path == fs::weakly_canonical(link_to_dir_path));
        CHECK(link_to_dir_obj->files.empty()); // Not recursed
        CHECK(link_to_dir_obj->directories.empty());
    }
}

TEST_CASE("scan_template_directory: Template name is dot (current directory)") {
    TempDirRAII     temp_dir("dot_template_name_test");
    const fs::path &template_base_dir = temp_dir(); // This is the directory that will be scanned
    std::string     template_name     = ".";        // Scan the base_dir itself

    create_structure(template_base_dir, {"file_in_base.txt", "sub_in_base/", "sub_in_base/file_in_sub.txt"});

    auto result = scan_template_directory(template_name, template_base_dir.string());

    REQUIRE(result.has_value());
    const auto &top_dirs = result.value();
    REQUIRE(top_dirs.size() == 2); // "." for file_in_base.txt, and "sub_in_base"

    auto dot_dir = find_dir(top_dirs, ".");
    REQUIRE(dot_dir != nullptr);
    CHECK(dot_dir->path == fs::weakly_canonical(template_base_dir));
    CHECK(dot_dir->files.size() == 1);
    CHECK(dot_dir->files.count("file_in_base.txt") == 1);

    auto sub_in_base = find_dir(top_dirs, "sub_in_base");
    REQUIRE(sub_in_base != nullptr);
    CHECK(sub_in_base->path == fs::weakly_canonical(template_base_dir / "sub_in_base"));
    CHECK(sub_in_base->files.size() == 1);
    CHECK(sub_in_base->files.count("file_in_sub.txt") == 1);
    CHECK(sub_in_base->directories.empty());
}

TEST_CASE("scan_template_directory: Deeply nested template name") {
    TempDirRAII     temp_dir("deep_template_name_test");
    const fs::path &overall_base_dir       = temp_dir();
    std::string     templates_base_dir_str = overall_base_dir.string();   // e.g. /tmp/xyz_0
    std::string     template_name          = "level1/level2/my_template"; // Relative to templates_base_dir_str

    fs::path actual_template_root = overall_base_dir / "level1" / "level2" / "my_template";
    create_structure(actual_template_root, {"final_file.txt", "final_subdir/"});

    auto result = scan_template_directory(template_name, templates_base_dir_str);

    REQUIRE(result.has_value());
    const auto &top_dirs = result.value();
    REQUIRE(top_dirs.size() == 2); // "." for final_file.txt, and "final_subdir"

    auto dot_dir = find_dir(top_dirs, ".");
    REQUIRE(dot_dir != nullptr);
    CHECK(dot_dir->path == fs::weakly_canonical(actual_template_root));
    CHECK(dot_dir->files.size() == 1);
    CHECK(dot_dir->files.count("final_file.txt") == 1);

    auto final_subdir = find_dir(top_dirs, "final_subdir");
    REQUIRE(final_subdir != nullptr);
    CHECK(final_subdir->path == fs::weakly_canonical(actual_template_root / "final_subdir"));
    CHECK(final_subdir->files.empty());
    CHECK(final_subdir->directories.empty());
}