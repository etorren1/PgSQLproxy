#include "ConnectionManager.hpp"

#include <unistd.h>
#include <arpa/inet.h>

namespace prx {

    ConnectionMagager::ConnectionMagager() {}
    ConnectionMagager::~ConnectionMagager() {}

    void ConnectionMagager::addUser(int userSocket, const sockaddr_in& address)
    {
        pollfd pollFd;
        pollFd.fd = userSocket;
        pollFd.events = POLLIN | POLLOUT;
        pollFd.revents = 0;
        fdPull_.push_back(std::move(pollFd));

        std::cout << "New client on " << userSocket << " socket." << "\n";

        int dbSocket;
        /* socket initialization */
        if ((dbSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Socket initialization failed");
        }
        struct sockaddr_in  dbAddres;
        dbAddres.sin_family = AF_INET;
        dbAddres.sin_port = htons(std::stoi("5432"));
        if (inet_pton(AF_INET, "127.0.0.1", &dbAddres.sin_addr) < 0)
        {
            perror("Unable IP translation to special numeric format");
        }
        /* binding IP and PORT to the server socket */
        if (connect(dbSocket, (sockaddr*)&dbAddres, sizeof(dbAddres)) != 0) {
            perror("Connect to database failed");
        }

        pollFd.fd = dbSocket;
        pollFd.events = POLLIN;
        pollFd.revents = 0;
        fdPull_.push_back(std::move(pollFd));

        userPull_.push_back(std::make_unique<User>(userSocket, dbSocket, inet_ntoa(address.sin_addr)));
        std::cout << "Connect to database on " << dbSocket << " socket." << "\n";
        userTable_.emplace(userSocket, userPull_.back().get());
        userTable_.emplace(dbSocket, userPull_.back().get());


    }

    void ConnectionMagager::eraseUser(int fd)
    {
        User* user = userTable_[fd];
        int clientFd = user->getClientFd();
        int dbFd = user->getDbFd();

        auto userIt = std::find_if(userPull_.begin(), userPull_.end(), 
                                [clientFd, dbFd](auto& u){ return u->getClientFd() == clientFd
                                                        && u->getDbFd() == dbFd; });
        if (userIt == userPull_.end()) {
            std::cout << "ConnectionMagager::eraseUser| Unknown user fd = " << fd << std::endl;
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
        std::cout << clientFd << " gone away.n";
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