#include "User.hpp"

namespace prx
{
    User::User(int userFd, int dbFd, char* ip)
        : userFd_(userFd), dbFd_(dbFd), ipv4_(ip)
    {
    }

    User::~User()
    {
        std::cout << "User destructor for User & DB";
        std::cout << userFd_ << " " << dbFd_ << std::endl;
    }

    int User::getClientFd() const
    {
        return userFd_;
    }

    int User::getDbFd() const
    {
        return dbFd_;
    }

    bool User::isRequest(int fd) const
    {
        return fd == userFd_ ? true : false;
    }

    const std::vector<char>& User::getResponceQuery()
    {
        return responceQuery_;
    }

    const std::vector<char>& User::getRequestQuery()
    {
        return requestQuery_;
    }

    /* void User::checkConnection()
    {
        if (message.find_last_of("\n") != message.size() - 1) {
            breakconnect_ = true;
        }
        else if (breakconnect_) {
            breakconnect_ = false;
        }
    }
 */
    /* bool User::getBreakconnect() const
    {
        return breakconnect_;
    } */

    void User::appendQuery(int fd, char* buf, int buf_size)
    {
        bool isRequest = fd == userFd_;
        std::vector<char>* vectorPtr = isRequest ? &requestQuery_ : &responceQuery_;

        /* if (vectorPtr->empty()) {
            if (enabledProtocol_v3)
        } */

        for (int i = 0; i < buf_size; i++) {
            vectorPtr->push_back(buf[i]);
        }
    }

    void User::clearRequest()
    {
        requestQuery_.clear();
    }

    void User::clearResponce()
    {
        responceQuery_.clear();
    }
    
} // namespace prx
