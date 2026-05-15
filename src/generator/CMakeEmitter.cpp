#include "generator/CMakeEmitter.h"

#include "generator/CMakeEmitter.h"

#include <sstream>

namespace visiform::generator {

std::string CMakeEmitter::emit(const model::ProjectDocument& document) const
{
    std::ostringstream stream;
    stream << "# Placeholder CMake generated for " << document.projectName << '\n';
    stream << "cmake_minimum_required(VERSION 3.24)\n";
    stream << "project(" << document.projectName << ")\n";
    return stream.str();
}

} // namespace visiform::generator
