// tests/placeholder_processor_tests.cpp
#include "cgen/placeholder_processor.h"

#include <doctest/doctest.h>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

using namespace cgen;

TEST_CASE("Extract placeholders with default style (AtSign)") {
    PlaceholderProcessor processor;

    // Test with single placeholder
    std::vector<std::string> result = processor.extractPlaceholders("@FOO@");
    CHECK(result.size() == 1);
    CHECK(result[0] == "FOO");

    // Test with multiple placeholders
    result = processor.extractPlaceholders("@FOO@\n@BAR@");
    CHECK(result.size() == 2);
    CHECK(result[0] == "FOO");
    CHECK(result[1] == "BAR");

    // Test with empty content
    result = processor.extractPlaceholders("");
    CHECK(result.empty());

    // Test with no placeholders
    result = processor.extractPlaceholders("Some text without placeholders");
    CHECK(result.empty());

    // Test with lowercase placeholder (should not be matched)
    result = processor.extractPlaceholders("@foo@");
    CHECK(result.empty());

    // Test with special characters in placeholder (only uppercase letters, numbers, underscores allowed)
    result = processor.extractPlaceholders("@FOO123_BAR@");
    CHECK(result.size() == 1);
    CHECK(result[0] == "FOO123_BAR");
}

TEST_CASE("Extract placeholders with multiple styles") {
    PlaceholderProcessor processor({PlaceholderStyle::AtSign, PlaceholderStyle::HashTag});

    // Test with both @ and # styles
    std::vector<std::string> result = processor.extractPlaceholders("@FOO@\n#BAR#");
    CHECK(result.size() == 2);
    CHECK(result[0] == "FOO");
    CHECK(result[1] == "BAR");

    // Test with mixed styles
    result = processor.extractPlaceholders("#FOO#\n@BAR@");
    CHECK(result.size() == 2);
    CHECK(result[0] == "FOO");
    CHECK(result[1] == "BAR");

    // Test with invalid style (should not match)
    result = processor.extractPlaceholders("$$FOO$$");
    CHECK(result.empty());
}

TEST_CASE("Replace placeholders with values") {
    PlaceholderProcessor processor;

    // Basic replacement
    std::string result = processor.replacePlaceholders("@FOO@", {{"FOO", "hello"}});
    CHECK(result == "hello");

    // Multiple placeholders
    result = processor.replacePlaceholders("@FOO@\n@BAR@", {{"FOO", "hello"}, {"BAR", "world"}});
    CHECK(result == "hello\nworld");

    // Placeholder not in values should remain unchanged
    result = processor.replacePlaceholders("@FOO@", {{"BAR", "hello"}});
    CHECK(result == "@FOO@");

    // Case sensitivity: lowercase not matched
    result = processor.replacePlaceholders("@foo@", {{"FOO", "hello"}});
    CHECK(result == "@foo@");

    // Placeholder with underscore and numbers
    result = processor.replacePlaceholders("@FOO_123@", {{"FOO_123", "value"}});
    CHECK(result == "value");
}

TEST_CASE("Edge cases for replace") {
    PlaceholderProcessor processor;

    // Empty content
    std::string result = processor.replacePlaceholders("", {});
    CHECK(result == "");

    // Empty values map
    result = processor.replacePlaceholders("@FOO@", {});
    CHECK(result == "@FOO@");

    // Replacing with empty string
    result = processor.replacePlaceholders("@FOO@", {{"FOO", ""}});
    CHECK(result == "");

    // Overlapping placeholders (sorted by length)
    result = processor.replacePlaceholders("@FOO_BAR@", {{"FOO_BAR", "value1"}, {"FOO", "value2"}});
    CHECK(result == "value1");
}

TEST_CASE("Replace with multiple styles") {
    PlaceholderProcessor processor({PlaceholderStyle::AtSign, PlaceholderStyle::HashTag});

    // Replace both @ and # styles
    std::string result = processor.replacePlaceholders("@FOO@\n#BAR#", {{"FOO", "hello"}, {"BAR", "world"}});
    CHECK(result == "hello\nworld");

    // Replace with only one style present
    result = processor.replacePlaceholders("#FOO#", {{"FOO", "hello"}});
    CHECK(result == "hello");

    // Replace with only one style in content
    result = processor.replacePlaceholders("@FOO@", {{"FOO", "hello"}});
    CHECK(result == "hello");

    // Placeholder not in values
    result = processor.replacePlaceholders("#FOO#", {{"BAR", "hello"}});
    CHECK(result == "#FOO#");
}