#ifndef QueryLogger_hpp
#define QueryLogger_hpp

#include "Utils.hpp"

#include <ctime>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

namespace prx
{
    class QueryLogger {

        public:
            QueryLogger(const std::string& name, const std::string& path);
            ~QueryLogger();

            void    writeLog(const std::string& message, const std::string& prefix = "");
            void    newLog();

        private:
            std::string getFilePostfix();
            std::string getTime();
            void        openFile();

            int fd_ = -1;
            std::string filePath_;
            std::string fileName_;
    };

} // namespace prx


#endif