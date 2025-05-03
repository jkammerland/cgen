module;

#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <fmt/core.h>
#include <fmt/format.h>

export module cgen.generator;

namespace fs = std::filesystem;

export namespace cgen {
    class Generator {
    public:
        Generator(const std::string& projectName, 
                  const std::string& projectVersion = "0.1.0",
                  const std::string& projectDescription = "A C++ Project") 
            : m_projectName(projectName),
              m_projectVersion(projectVersion),
              m_projectDescription(projectDescription),
              m_templateDir("template") {
            // Initialize default mappings
            m_mappings["PROJECT_NAME"] = m_projectName;
            m_mappings["PROJECT_NAME_UPPER"] = toUpper(m_projectName);
            m_mappings["PROJECT_VERSION"] = m_projectVersion;
            m_mappings["PROJECT_DESCRIPTION"] = m_projectDescription;
            m_mappings["PROJECT_VENDOR"] = "Your Organization";
            m_mappings["PROJECT_CONTACT"] = "your.email@example.com";
            m_mappings["NAMESPACE"] = m_projectName;
            m_mappings["MODULE_NAME"] = "core";
            m_mappings["CLASS_NAME"] = "Core";
            m_mappings["DEPENDENCIES"] = "";
            m_mappings["PROJECT_DEPENDENCIES"] = "";
            m_mappings["FIND_DEPENDENCIES"] = "";
        }
        
        ~Generator() = default;
        
        void setNamespace(const std::string& ns) {
            m_mappings["NAMESPACE"] = ns;
        }
        
        void setModuleName(const std::string& moduleName) {
            m_mappings["MODULE_NAME"] = moduleName;
        }
        
        void setClassName(const std::string& className) {
            m_mappings["CLASS_NAME"] = className;
        }
        
        void addDependency(const std::string& depName) {
            std::string deps = m_mappings["DEPENDENCIES"];
            deps += fmt::format("find_package({} REQUIRED)\n", depName);
            m_mappings["DEPENDENCIES"] = deps;
            
            std::string projDeps = m_mappings["PROJECT_DEPENDENCIES"];
            if (!projDeps.empty()) {
                projDeps += " ";
            }
            projDeps += fmt::format("{}::{}",depName, depName);
            m_mappings["PROJECT_DEPENDENCIES"] = projDeps;
            
            std::string findDeps = m_mappings["FIND_DEPENDENCIES"];
            findDeps += fmt::format("find_dependency({} REQUIRED)\n", depName);
            m_mappings["FIND_DEPENDENCIES"] = findDeps;
        }
        
        void setMapping(const std::string& key, const std::string& value) {
            m_mappings[key] = value;
        }
        
        void generate(const fs::path& outputDir) {
            // Create project directories
            fs::create_directories(outputDir);
            fs::create_directories(outputDir / "src");
            fs::create_directories(outputDir / "include" / m_projectName);
            fs::create_directories(outputDir / "cmake");
            
            // Process templates
            processTemplate("root_CMakeLists.txt.template", outputDir / "CMakeLists.txt");
            processTemplate("src_CMakeLists.txt.template", outputDir / "src" / "CMakeLists.txt");
            processTemplate("module.cppm.template", outputDir / "src" / 
                          (m_mappings["MODULE_NAME"] + ".cppm"));
            processTemplate("main.cpp.template", outputDir / "src" / "main.cpp");
            processTemplate("config.cmake.in.template", outputDir / "cmake" / 
                          (m_projectName + "-config.cmake.in"));
        }
        
    private:
        std::string m_projectName;
        std::string m_projectVersion;
        std::string m_projectDescription;
        fs::path m_templateDir;
        std::map<std::string, std::string> m_mappings;
        
        std::string toUpper(const std::string& input) {
            std::string result = input;
            for (auto& c : result) {
                c = std::toupper(c);
            }
            return result;
        }
        
        void processTemplate(const fs::path& templateFile, const fs::path& outputFile) {
            // Read template content
            std::ifstream inFile(m_templateDir / templateFile);
            if (!inFile) {
                throw std::runtime_error(
                    fmt::format("Failed to open template file: {}", templateFile.string()));
            }
            
            std::string content((std::istreambuf_iterator<char>(inFile)),
                              std::istreambuf_iterator<char>());
            inFile.close();
            
            // Apply substitutions
            for (const auto& [key, value] : m_mappings) {
                // Create search pattern like {KEY}
                std::string pattern = "{" + key + "}";
                
                // Replace all occurrences
                size_t pos = 0;
                while ((pos = content.find(pattern, pos)) != std::string::npos) {
                    content.replace(pos, pattern.length(), value);
                    pos += value.length();
                }
            }
            
            // Write output file
            fs::create_directories(outputFile.parent_path());
            std::ofstream outFile(outputFile);
            if (!outFile) {
                throw std::runtime_error(
                    fmt::format("Failed to create output file: {}", outputFile.string()));
            }
            
            outFile << content;
            outFile.close();
        }
    };
}