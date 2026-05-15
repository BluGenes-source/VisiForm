#include "generator/CodeGenerator.h"

#include "generator/CodeGenerator.h"

#include "utils/FileUtils.h"

namespace visiform::generator {

bool CodeGenerator::generateProject(
    const model::ProjectDocument& document,
    const std::filesystem::path& outputDirectory,
    std::string& errorMessage) const
{
    errorMessage.clear();

    const std::filesystem::path sourceDirectory = outputDirectory / "src";
    if (!utils::FileUtils::ensureDirectoryExists(outputDirectory, errorMessage)) {
        return false;
    }
    if (!utils::FileUtils::ensureDirectoryExists(sourceDirectory, errorMessage)) {
        return false;
    }

    const auto emittedSources = visageCppEmitter_.emitProjectSources(document);
    if (!utils::FileUtils::writeTextFile(outputDirectory / "CMakeLists.txt", cmakeEmitter_.emitCMakeLists(document), errorMessage)) {
        return false;
    }
    if (!utils::FileUtils::writeTextFile(sourceDirectory / "main.cpp", emittedSources.mainCpp, errorMessage)) {
        return false;
    }
    if (!utils::FileUtils::writeTextFile(sourceDirectory / "MainWindow.h", emittedSources.mainWindowHeader, errorMessage)) {
        return false;
    }
    if (!utils::FileUtils::writeTextFile(sourceDirectory / "MainWindow.cpp", emittedSources.mainWindowCpp, errorMessage)) {
        return false;
    }

    return true;
}

} // namespace visiform::generator
