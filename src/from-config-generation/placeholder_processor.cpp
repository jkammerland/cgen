#include "cgen/placeholder_processor.h"
#include <sstream>
#include <unordered_set>

namespace cgen {

PlaceholderProcessor::PlaceholderProcessor(std::initializer_list<PlaceholderStyle> styles)
    : allStyles_(styles.begin(), styles.end()) {
}

std::vector<std::string> PlaceholderProcessor::extractPlaceholders(const std::string& content) const {
    std::vector<std::string> placeholders;
    std::unordered_set<std::string> uniquePlaceholders; // To avoid duplicates
    
    auto regex = buildCombinedRegex();
    
    std::sregex_iterator begin(content.begin(), content.end(), regex);
    std::sregex_iterator end;
    
    for (std::sregex_iterator i = begin; i != end; ++i) {
        std::smatch match = *i;
        std::string placeholder = extractPlaceholderName(match.str());
        
        if (!placeholder.empty() && uniquePlaceholders.find(placeholder) == uniquePlaceholders.end()) {
            placeholders.push_back(placeholder);
            uniquePlaceholders.insert(placeholder);
        }
    }
    
    return placeholders;
}

std::string PlaceholderProcessor::replacePlaceholders(
    const std::string& content,
    const std::unordered_map<std::string, std::string>& values
) const {
    std::string result = content;
    auto regex = buildCombinedRegex();
    
    // First pass: collect all placeholders with their full match text
    std::vector<std::pair<std::string, std::string>> replacements;
    
    std::sregex_iterator begin(content.begin(), content.end(), regex);
    std::sregex_iterator end;
    
    for (std::sregex_iterator i = begin; i != end; ++i) {
        std::smatch match = *i;
        std::string fullMatch = match.str();
        std::string placeholder = extractPlaceholderName(fullMatch);
        
        auto it = values.find(placeholder);
        if (it != values.end()) {
            replacements.emplace_back(fullMatch, it->second);
        }
    }
    
    // Second pass: apply replacements from longest to shortest to avoid partial replacements
    std::sort(replacements.begin(), replacements.end(), 
              [](const auto& a, const auto& b) { 
                  return a.first.length() > b.first.length(); 
              });
    
    for (const auto& [oldText, newText] : replacements) {
        // Use simple string replacement for exact matches
        size_t pos = 0;
        while ((pos = result.find(oldText, pos)) != std::string::npos) {
            result.replace(pos, oldText.length(), newText);
            pos += newText.length();
        }
    }
    
    return result;
}

std::regex PlaceholderProcessor::buildRegexForStyle(PlaceholderStyle style) const {
    auto [prefix, suffix] = getStyleDelimiters(style);
    
    // Escape prefix and suffix for regex
    auto escapeRegex = [](const std::string& str) {
        std::string escaped;
        for (char c : str) {
            if (c == '{' || c == '}' || c == '$' || c == '@' || c == '#' || c == '%' || c == '\\') {
                escaped += '\\';
            }
            escaped += c;
        }
        return escaped;
    };
    
    std::string pattern = escapeRegex(prefix) + "([A-Z0-9_]+)" + escapeRegex(suffix);
    return std::regex(pattern);
}

std::pair<std::string, std::string> PlaceholderProcessor::getStyleDelimiters(PlaceholderStyle style) const {
    switch (style) {
        case PlaceholderStyle::AtSign:
            return {"@", "@"};
        case PlaceholderStyle::HashTag:
            return {"#", "#"};
        case PlaceholderStyle::Percent:
            return {"%", "%"};
        default:
            return {"{", "}"};
    }
}

std::regex PlaceholderProcessor::buildCombinedRegex() const {
    // Build pattern alternation for all styles
    std::stringstream pattern;
    
    for (size_t i = 0; i < allStyles_.size(); ++i) {
        auto [prefix, suffix] = getStyleDelimiters(allStyles_[i]);
        
        // Escape prefix and suffix for regex
        auto escapeRegex = [](const std::string& str) {
            std::string escaped;
            for (char c : str) {
                if (c == '{' || c == '}' || c == '$' || c == '@' || c == '#' || c == '%' || c == '\\') {
                    escaped += '\\';
                }
                escaped += c;
            }
            return escaped;
        };
        
        pattern << escapeRegex(prefix) << "([A-Z0-9_]+)" << escapeRegex(suffix);
        
        if (i < allStyles_.size() - 1) {
            pattern << "|";
        }
    }
    
    return std::regex(pattern.str());
}

std::string PlaceholderProcessor::extractPlaceholderName(const std::string& match) const {
    // Try each style to extract the placeholder name
    for (auto style : allStyles_) {
        auto [prefix, suffix] = getStyleDelimiters(style);
        
        if (match.size() >= prefix.size() + suffix.size() && 
            match.substr(0, prefix.size()) == prefix && 
            match.substr(match.size() - suffix.size()) == suffix) {
            
            return match.substr(prefix.size(), match.size() - prefix.size() - suffix.size());
        }
    }
    
    return "";
}

} // namespace cgen