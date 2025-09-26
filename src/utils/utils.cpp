#include "utils/utils.hpp"

namespace utils
{
bool writeFile(const std::string& content, const char* path) {
    std::ofstream out(path, std::ios::binary);

    if (!out.is_open()){
        return false;
    }
    out.write(content.data(), content.size());
    out.close();
    return out.good();
}
} // namespace utils