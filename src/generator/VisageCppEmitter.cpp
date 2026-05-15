#include "generator/VisageCppEmitter.h"

#include "generator/VisageCppEmitter.h"

#include <sstream>

namespace visiform::generator {

std::string VisageCppEmitter::emit(const model::ProjectDocument& document) const
{
    std::ostringstream stream;
    stream << "// Placeholder Visage source for " << document.projectName << '\n';
    stream << "// Main form: " << document.mainFormClassName << '\n';
    stream << "// Root widget: " << document.root.typeName() << '\n';
    return stream.str();
}

} // namespace visiform::generator
