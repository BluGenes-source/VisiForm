#include "utils/IdGenerator.h"

#include <sstream>

namespace visiform::utils {

std::string IdGenerator::next(const std::string& prefix)
{
    std::ostringstream stream;
    stream << prefix << nextValue_++;
    return stream.str();
}

} // namespace visiform::utils
