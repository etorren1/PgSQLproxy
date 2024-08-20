#include "Utils.hpp"

#include <sys/stat.h>

namespace utils
{
    void	rec_mkdir(std::string path)
    {
        size_t sep = path.find_last_of("/");
        std::string create = path;
        if (sep != std::string::npos) {
            rec_mkdir(path.substr(0, sep));
            path.erase(0, sep);
        }
        mkdir(create.c_str(), 0755);
    }

} // namespace utils