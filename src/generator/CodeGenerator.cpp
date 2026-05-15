#include "generator/CodeGenerator.h"

#include "utils/FileUtils.h"

#include <array>

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
    std::string& errorMessage) const
{
    errorMessage.clear();

    if (!utils::FileUtils::ensureDirectoryExists(outputDirectory, errorMessage)) {
        return false;
    }

    const auto emittedSources = visageCppEmitter_.emitProjectSources(document);
    if (!writeGeneratedFile(outputDirectory, "CMakeLists.txt", cmakeEmitter_.emitCMakeLists(document), errorMessage)) {
        return false;
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
    if (!writeGeneratedFile(outputDirectory, std::filesystem::path{"src"} / "MainWindow.h", emittedSources.mainWindowHeader, errorMessage)) {
        return false;
    }
    if (!writeGeneratedFile(outputDirectory, std::filesystem::path{"src"} / "MainWindow.cpp", emittedSources.mainWindowCpp, errorMessage)) {
        return false;
    }

    return true;
}

} // namespace visiform::generator
