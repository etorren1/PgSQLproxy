#include "ConnectionManager.hpp"

#include <unistd.h>
#include <arpa/inet.h>

namespace prx {

    ConnectionMagager::ConnectionMagager() {}
    ConnectionMagager::~ConnectionMagager() {}

    int ConnectionMagager::createDbSocket()
    {
        int sock;
        /* socket initialization */
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Socket initialization failed");
            return -1;
        }
        struct sockaddr_in  addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(std::stoi("5432"));
        if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) < 0)
        {
            perror("Unable IP translation to special numeric format");
            close(sock);
            return -1;
        }
        /* binding IP and PORT to the database socket */
        if (connect(sock, (sockaddr*)&addr, sizeof(addr)) != 0) {
            perror("Connect to database failed");
            close(sock);
            return -1;
        }
        return sock;
    }

    bool ConnectionMagager::checkDbConnection()
    {
        int sock = createDbSocket();
        if (createDbSocket() == -1) {
            return false;
        }
        close(sock);
        return true;
    }

    void ConnectionMagager::addUser(int userSocket, const sockaddr_in& address)
    {
        pollfd pollFd;
        pollFd.fd = userSocket;
        pollFd.events = POLLIN | POLLOUT;
        pollFd.revents = 0;
        fdPull_.push_back(std::move(pollFd));

        std::cout << "New client on " << userSocket << " socket." << "\n";

        int dbSocket = createDbSocket();
        pollFd.fd = dbSocket;
        pollFd.events = POLLIN;
        pollFd.revents = 0;
        fdPull_.push_back(std::move(pollFd));

        userPull_.push_back(std::make_unique<User>(userSocket, dbSocket, inet_ntoa(address.sin_addr), ntohs(address.sin_port)));
        std::cout << "Connect to database on " << dbSocket << " socket." << "\n";
        userTable_.emplace(userSocket, userPull_.back().get());
        userTable_.emplace(dbSocket, userPull_.back().get());


    }

    void ConnectionMagager::eraseUser(User& user)
    {
        int clientFd = user.getClientFd();
        int dbFd = user.getDbFd();

        auto userIt = std::find_if(userPull_.begin(), userPull_.end(), 
                                [clientFd, dbFd](auto& u){ return u->getClientFd() == clientFd
                                                        && u->getDbFd() == dbFd; });
        if (userIt == userPull_.end()) {
            std::cout << "ConnectionMagager::eraseUser| Unknown user" << std::endl;
            exit(1);
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
        std::cout << clientFd << " gone away\n";
    }

    User& ConnectionMagager::getUser(int fd)
    {
        auto it = userTable_.find(fd);
        if (it == userTable_.end()) {
            std::cout << "ConnectionMagager::getUser| Unknown user fd = " << fd << std::endl;
            exit(1);
        }
        return *it->second;
    }

    std::vector<struct pollfd>& ConnectionMagager::getFds()
    {
        return fdPull_;
    }

    void ConnectionMagager::closeAll()
    {
        for (auto elem : fdPull_) {
            close(elem.fd);
        }
    }

}