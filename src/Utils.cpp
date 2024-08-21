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

    int     bytesToInteger(const char * buffer)
    {
        int size = static_cast<int>(static_cast<unsigned char>(buffer[0]) << 24 |
            static_cast<unsigned char>(buffer[1]) << 16 | 
            static_cast<unsigned char>(buffer[2]) << 8 | 
            static_cast<unsigned char>(buffer[3]));
        return size;
    }

    std::string	    trim(std::string str, std::string cut)
    {
        if (!cut.size())
            return str;
        size_t beg = str.find_first_not_of(cut);
        size_t end = str.find_last_not_of(cut);
        if (beg == std::string::npos || end == std::string::npos)
            return "";
        return str.substr(beg, end - beg + 1);
    }

} // namespace utils