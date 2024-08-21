#include "ConnectionManager.hpp"

#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

namespace prx {

    ConnectionMagager::ConnectionMagager(const std::string& host, int port) 
        : dbInfo_({host, port})
    {
    }

    ConnectionMagager::~ConnectionMagager()
    {
    }

    int     ConnectionMagager::createDbSocket()
    {
        int sock;
        /* Socket initialization */
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            return ERROR;
        }
        struct sockaddr_in  addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(dbInfo_.port);
        if (inet_pton(AF_INET, dbInfo_.hostname.c_str(), &addr.sin_addr) < 0)
        {
            close(sock);
            return ERROR;
        }
        /* Binding IP and PORT to the database socket */
        if (connect(sock, (sockaddr*)&addr, sizeof(addr)) != 0) {
            close(sock);
            return ERROR;
        }
        return sock;
    }

    bool    ConnectionMagager::checkDbConnection()
    {
        int sock = createDbSocket();
        if (sock == UNKNOWN_FD) {
            return false;
        }
        return true;
    }

    void    ConnectionMagager::addUser(int userSocket, const sockaddr_in& address)
    {
        int dbSocket = createDbSocket();
        if (dbSocket == UNKNOWN_FD) {
            std::cout << "Could not create connection to database for new user " << userSocket << std::endl;
            close(userSocket);
            return ;
        }

        struct pollfd pollFd;
        /* init pollfd for user socket */
        pollFd.fd = userSocket;
        pollFd.events = POLLIN ;
        pollFd.revents = 0;
        fdPull_.push_back(std::move(pollFd));

        /* init pollfd for database socket */
        pollFd.fd = dbSocket;
        pollFd.events = POLLIN;
        pollFd.revents = 0;
        fdPull_.push_back(std::move(pollFd));

        userPull_.push_back(std::make_unique<User>(userSocket, dbSocket, inet_ntoa(address.sin_addr), ntohs(address.sin_port)));
        userTable_.emplace(userSocket, userPull_.back().get());
        userTable_.emplace(dbSocket, userPull_.back().get());

        fcntl(userSocket, F_SETFL, O_NONBLOCK);
        fcntl(dbSocket, F_SETFL, O_NONBLOCK);
    }

    void    ConnectionMagager::removeUser(User& user)
    {
        int clientFd = user.getClientFd();
        int dbFd = user.getDbFd();

        auto userIt = std::find_if(userPull_.begin(), userPull_.end(), 
                                [clientFd, dbFd](auto& u){ return u->getClientFd() == clientFd
                                                        && u->getDbFd() == dbFd; });
        if (userIt == userPull_.end()) {
            throw std::logic_error("Removing unknown user");
        }
        int count = 0;
        for (auto it = fdPull_.begin(); it != fdPull_.end(); ) {
            if (it->fd == clientFd || it->fd == dbFd) {
                it = fdPull_.erase(it);
                count++;
            } else it++;
            if (count == 2) break; // stop iteraiting after removing both connection
        }
        count = 0;
        for (auto it = userTable_.begin(); it != userTable_.end(); ) {
            if (it->first == clientFd || it->first == dbFd) {
                it = userTable_.erase(it);
                count++;
            }
            else it++;
            if (count == 2) break; // stop iteraiting after removing both connection
        }
        close(clientFd);
        close(dbFd);
        userPull_.erase(userIt);
    }

    User&   ConnectionMagager::getUser(int fd)
    {
        auto it = userTable_.find(fd);
        if (it == userTable_.end()) {
            throw std::logic_error("Getting unknown user");
        }
        return *it->second;
    }

    std::vector<struct pollfd>& ConnectionMagager::getFds()
    {
        return fdPull_;
    }

    void    ConnectionMagager::closeAll()
    {
        for (auto elem : fdPull_) {
            close(elem.fd);
        }
    }

    void    ConnectionMagager::logUserList() const
    {
        for (auto& user : userPull_) {
            std::cout << "   " << user->getAppInfo() 
             << " | " << user->getClientFd()
             << " | " << user->getDbFd() 
             << std::endl;
        }
    }

    size_t  ConnectionMagager::getUsersCount() const
    {
        return userPull_.size();
    }
}