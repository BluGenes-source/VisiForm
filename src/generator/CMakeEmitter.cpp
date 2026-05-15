#include "generator/CMakeEmitter.h"

#include <sstream>

namespace visiform::generator {

std::string CMakeEmitter::emit(const model::ProjectDocument& document) const
{
    std::ostringstream stream;
    stream << "# Placeholder CMake generated for " << document.name() << '\n';
    stream << "cmake_minimum_required(VERSION 3.24)\n";
    stream << "project(" << document.name() << ")\n";
    return stream.str();
}

} // namespace visiform::generator
