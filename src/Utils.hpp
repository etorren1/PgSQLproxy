#ifndef Utils_hpp
#define Utils_hpp

#include <iostream>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <map>

/* Postgres constants */
#define PqMsg_Query         'Q'
#define PqMsg_Terminate     'X'
#define PG_PROTOCOL(m,n)    (((m) << 16) | (n))
#define NEGOTIATE_SSL_CODE PG_PROTOCOL(1234,5679)

namespace prx
{
    static const int ERROR      = -1;
    static const int UNKNOWN_FD = -1;
    static const int STOP       = 0;
    static const int RESTART    = 1;
    static const int WORKING    = 2;
    static const int BACKLOG    = 100;      // Count of connections at one time
    static const int BUF_SIZE   = 65536; // Max count of readed bytes per recv()
} // namespace prx

struct config {
    std::string srvhost = "127.0.0.1";
    std::string srvport = "4242";
    std::string dbhost = "127.0.0.1";
    std::string dbport = "5432";
    std::string logpath = "logs";
    std::string logname = "Proxy";
};

namespace utils
{
    void	rec_mkdir(std::string path);
    int     bytesToInteger(const char * buffer);
    std::string	    trim(std::string str, std::string cut);

} // namespace utils

#endif 