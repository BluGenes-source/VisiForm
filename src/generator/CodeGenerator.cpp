#include "generator/CodeGenerator.h"

#include "utils/FileUtils.h"

#include <array>
#include <cctype>

namespace visiform::generator {
namespace {

bool isPathInsideOutputDirectory(const std::filesystem::path& outputDirectory, const std::filesystem::path& targetPath)
{
    const std::filesystem::path normalizedOutput = outputDirectory.lexically_normal();
    const std::filesystem::path normalizedTarget = targetPath.lexically_normal();
    const std::filesystem::path relative = normalizedTarget.lexically_relative(normalizedOutput);
    if (relative.empty()) {
        return false;
    }

    const std::string relativeText = relative.generic_string();
    return relativeText == "." || !relativeText.starts_with("..");
}

std::string sanitizeClassName(const std::string& value)
{
    std::string sanitized;
    sanitized.reserve(value.size());
    for (char character : value) {
        if (std::isalnum(static_cast<unsigned char>(character)) != 0 || character == '_') {
            sanitized.push_back(character);
        }
    }

    if (sanitized.empty()) {
        return "AppMainWindow";
    }
    if (std::isdigit(static_cast<unsigned char>(sanitized.front())) != 0) {
        sanitized.insert(sanitized.begin(), '_');
    }
    return sanitized == "MainWindow" ? std::string{"AppMainWindow"} : sanitized;
}

bool writeGeneratedFile(const std::filesystem::path& outputDirectory,
    const std::filesystem::path& relativePath,
    const std::string& content,
    std::string& errorMessage)
{
    if (relativePath.is_absolute()) {
        errorMessage = "Generated file path must be relative: " + relativePath.string();
        return false;
    }

    const std::filesystem::path targetPath = (outputDirectory / relativePath).lexically_normal();
    if (!isPathInsideOutputDirectory(outputDirectory, targetPath)) {
        errorMessage = "Refusing to write outside export folder: " + targetPath.string();
        return false;
    }

    if (!targetPath.parent_path().empty() && !utils::FileUtils::ensureDirectoryExists(targetPath.parent_path(), errorMessage)) {
        return false;
    }

    return utils::FileUtils::writeTextFile(targetPath, content, errorMessage);
}

} // namespace

bool CodeGenerator::generateProject(
    const model::ProjectDocument& document,
    const std::filesystem::path& outputDirectory,
    std::string& errorMessage,
    ProgressCallback progressCallback) const
{
    errorMessage.clear();

    if (!utils::FileUtils::ensureDirectoryExists(outputDirectory, errorMessage)) {
        return false;
    }

    if (progressCallback) {
        progressCallback(0, "Preparing export");
    }

    std::string existingMainWindowCpp;
    std::string readErrorMessage;
    const std::string userSubclassName = sanitizeClassName(document.userSubclassName.empty() ? document.mainFormClassName : document.userSubclassName);
    const std::filesystem::path existingMainWindowCppPath = outputDirectory / "src" / (userSubclassName + ".cpp");
    if (std::filesystem::exists(existingMainWindowCppPath)) {
        utils::FileUtils::readTextFile(existingMainWindowCppPath, existingMainWindowCpp, readErrorMessage);
    }

    VisageCppEmitter::EmittedSources emittedSources;
    if (progressCallback) {
        progressCallback(20, "Validating document");
    }
    if (!visageCppEmitter_.emitProjectSources(document, existingMainWindowCpp, emittedSources, errorMessage)) {
        return false;
    }
    if (progressCallback) {
        progressCallback(40, "Generating sources");
    }
    if (!writeGeneratedFile(outputDirectory, "CMakeLists.txt", cmakeEmitter_.emitCMakeLists(document), errorMessage)) {
        return false;
    }
    if (progressCallback) {
        progressCallback(60, "Writing CMake files");
    }
    if (!writeGeneratedFile(outputDirectory, "CMakePresets.json", cmakeEmitter_.emitCMakePresets(), errorMessage)) {
        return false;
    }
    if (!writeGeneratedFile(outputDirectory, "README.md", cmakeEmitter_.emitReadme(document), errorMessage)) {
        return false;
    }
    if (!writeGeneratedFile(outputDirectory, ".gitignore", cmakeEmitter_.emitGitIgnore(), errorMessage)) {
        return false;
    }
    if (!writeGeneratedFile(outputDirectory, std::filesystem::path{"scripts"} / "configure_static_debug.cmd",
            cmakeEmitter_.emitConfigureScript(false), errorMessage)) {
        return false;
    }
    if (!writeGeneratedFile(outputDirectory, std::filesystem::path{"scripts"} / "build_static_debug.cmd",
            cmakeEmitter_.emitBuildScript(false), errorMessage)) {
        return false;
    }
    if (!writeGeneratedFile(outputDirectory, std::filesystem::path{"scripts"} / "configure_static_release.cmd",
            cmakeEmitter_.emitConfigureScript(true), errorMessage)) {
        return false;
    }
    if (!writeGeneratedFile(outputDirectory, std::filesystem::path{"scripts"} / "build_static_release.cmd",
            cmakeEmitter_.emitBuildScript(true), errorMessage)) {
        return false;
    }
    if (!writeGeneratedFile(outputDirectory, std::filesystem::path{"src"} / "main.cpp", emittedSources.mainCpp, errorMessage)) {
        return false;
    }
    if (progressCallback) {
        progressCallback(80, "Writing source files");
    }
    if (!writeGeneratedFile(outputDirectory, std::filesystem::path{"src"} / emittedSources.generatedBaseHeaderFilename, emittedSources.generatedBaseHeader, errorMessage)) {
        return false;
    }
    if (!writeGeneratedFile(outputDirectory, std::filesystem::path{"src"} / emittedSources.generatedBaseCppFilename, emittedSources.generatedBaseCpp, errorMessage)) {
        return false;
    }
    if (!writeGeneratedFile(outputDirectory, std::filesystem::path{"src"} / emittedSources.userSubclassHeaderFilename, emittedSources.userSubclassHeader, errorMessage)) {
        return false;
    }
    if (!writeGeneratedFile(outputDirectory, std::filesystem::path{"src"} / emittedSources.userSubclassCppFilename, emittedSources.userSubclassCpp, errorMessage)) {
        return false;
    }

    if (progressCallback) {
        progressCallback(100, "Export complete");
    }
    return true;
}

} // namespace visiform::generator
