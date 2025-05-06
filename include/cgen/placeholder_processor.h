#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <regex>

namespace cgen {

// Different placeholder format styles
enum class PlaceholderStyle {
    // Braces,       // {PLACEHOLDER}  # Hard to parse correctly
    // Dollar,       // $PLACEHOLDER   # Conflicts with bash
    // DollarBraces, // ${PLACEHOLDER} # Conflicts with cmake
    AtSign,       // @PLACEHOLDER@
    HashTag,      // #PLACEHOLDER#
    Percent       // %PLACEHOLDER%
};

class PlaceholderProcessor {
public:
    // Create processor with default style or specified style
    explicit PlaceholderProcessor(std::initializer_list<PlaceholderStyle> styles = {PlaceholderStyle::AtSign});
    
    // Extract all placeholders from a template
    std::vector<std::string> extractPlaceholders(const std::string& content) const;
    
    // Replace placeholders in content with values
    std::string replacePlaceholders(
        const std::string& content,
        const std::unordered_map<std::string, std::string>& values
    ) const;

private:
    std::vector<PlaceholderStyle> allStyles_;
    
    // Build regex for a specific style
    std::regex buildRegexForStyle(PlaceholderStyle style) const;
    
    // Get the prefix and suffix for a style
    std::pair<std::string, std::string> getStyleDelimiters(PlaceholderStyle style) const;
    
    // Build a combined regex for all active styles
    std::regex buildCombinedRegex() const;
    
    // Extract placeholder name from a matched string based on style
    std::string extractPlaceholderName(const std::string& match) const;
};

} // namespace cgen