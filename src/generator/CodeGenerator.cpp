#include "generator/CodeGenerator.h"

namespace visiform::generator {

std::string CodeGenerator::generateSummary(const model::ProjectDocument& document) const
{
    return visageCppEmitter_.emit(document) + "\n" + cmakeEmitter_.emit(document);
}

} // namespace visiform::generator
