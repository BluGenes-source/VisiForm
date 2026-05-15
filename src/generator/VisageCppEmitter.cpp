#include "generator/VisageCppEmitter.h"

#include <sstream>

namespace visiform::generator {

std::string VisageCppEmitter::emit(const model::ProjectDocument& document) const
{
    std::ostringstream stream;
    stream << "// Placeholder Visage source for " << document.name() << '\n';
    stream << "// Forms: " << document.forms().size() << '\n';
    return stream.str();
}

} // namespace visiform::generator
