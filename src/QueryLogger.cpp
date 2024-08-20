#include "QueryLogger.hpp"

namespace prx
{
    QueryLogger::QueryLogger(const std::string& name, const std::string& path)
        : fileName_(name), filePath_(path)
    {
    }

    QueryLogger::~QueryLogger()
    {
        if (fd_ > 0) {
            close(fd_);
        }
    }

    void QueryLogger::writeLog(const std::string& text, const std::string& prefix)
    {
        if (fd_ < 0) {
            openFile();
        }
        std::string time = getTime();
        write(fd_, time.c_str(), time.size());
        if (!prefix.empty()) {
            write(fd_, " | ", 3);
            write(fd_, prefix.c_str(), prefix.size());
        }
        write(fd_, " > ", 3);
        write(fd_, text.c_str(), text.size());
        if (text.back() != '\n')
            write(fd_, "\n", 1);
    }

    void QueryLogger::newLog()
    {
        if (fd_ > 0) {
            close(fd_);
        }
        openFile();
    }

    std::string QueryLogger::getFilePostfix()
    {
        std::time_t t = std::time(0);
        std::tm* tm = std::localtime(&t);
        std::string date = '-' + std::to_string(tm->tm_year + 1900) + '-' 
            + std::to_string(tm->tm_mon + 1) + '-' +  std::to_string(tm->tm_mday) + '-'
            + std::to_string(tm->tm_hour) + std::to_string(tm->tm_sec) + ".log";
        return date;
    }

    std::string QueryLogger::getTime()
    {
        std::time_t t = std::time(0);
        std::tm* tm = std::localtime(&t);
        char time[8];
        sprintf(time, "%0*d:%0*d:%0*d", 2, tm->tm_hour, 2, tm->tm_min, 2, tm->tm_sec);
        return time;
    }

    void QueryLogger::openFile()
    {
        std::string fullName;
        if (!filePath_.empty()) {
            fullName += filePath_ + '/';
        }
        fullName += fileName_ + getFilePostfix();

        int flags = O_RDWR | O_CREAT | O_APPEND;
        fd_ = open(fullName.c_str(), flags, 0644);
        if (fd_ < 0) {
            if (!filePath_.empty()) {
                utils::rec_mkdir(filePath_.c_str());
            }
            fd_ = open(fullName.c_str(), flags, 0644);
            if (fd_ < 0) {
                std::cerr << "Error: can not open or create log file\n";
                return ;
            }
        }
    }

} // namespace prx
