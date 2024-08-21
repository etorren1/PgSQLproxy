#ifndef QueryLogger_hpp
#define QueryLogger_hpp

#include "Utils.hpp"

#include <ctime>
#include <fcntl.h>
#include <unistd.h>

namespace prx
{
    class QueryLogger {

        public:
            QueryLogger(const std::string& name, const std::string& path);
            ~QueryLogger();

            void    writeLog(const std::string& message, const std::string& prefix = "");
            bool    newLog(); //! Create new log file. Return false if error occured

        private:
            std::string getFilePostfix();
            std::string getTime();
            int         openFile();

            int fd_ = UNKNOWN_FD;
            std::string fileName_;
            std::string filePath_;
    };

} // namespace prx


#endif