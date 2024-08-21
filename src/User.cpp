#include "User.hpp"

namespace prx
{
    User::User(int userFd, int dbFd, char* ip, uint16_t port)
        : userFd_(userFd), dbFd_(dbFd), ipv4_(ip), port_(port)
    {
    }

    User::~User()
    {
    }

    int     User::getClientFd() const
    {
        return userFd_;
    }

    int     User::getDbFd() const
    {
        return dbFd_;
    }

    bool    User::isRequest(int fd) const
    {
        return fd == userFd_ ? true : false;
    }

    const std::vector<char>& User::getResponceQuery() const
    {
        return responceQuery_;
    }

    const std::vector<char>& User::getRequestQuery() const
    {
        return requestQuery_;
    }

    void    User::setAppInfo(const std::string& app, const std::string& login, const std::string& /*dbname*/)
    {
        appInfo_ = ipv4_ + ':' + std::to_string(port_);
        if (!app.empty()) {
            appInfo_ += " | " + app;
        }
        if (!login.empty()) {
            appInfo_ += " | " + login;
        }
    }

    std::string     User::getAppInfo() const
    {
        return appInfo_;
    }

    void    User::appendQuery(int fd, char* buf, int buf_size)
    {
        bool isRequest = fd == userFd_;
        std::vector<char>* vectorPtr = isRequest ? &requestQuery_ : &responceQuery_;
        for (int i = 0; i < buf_size; i++) {
            vectorPtr->push_back(buf[i]);
        }
    }

    void    User::clearRequest()
    {
        requestQuery_.clear();
    }

    void    User::clearResponce()
    {
        responceQuery_.clear();
    }

} // namespace prx
