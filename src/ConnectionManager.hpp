#ifndef ConnectionMagager_hpp
#define ConnectionMagager_hpp

#include <poll.h>
#include <map>
#include <algorithm>
#include <netinet/in.h>
#include <list>
#include <memory>

#include "User.hpp"

namespace prx {

    class ConnectionMagager {

        public:
            ConnectionMagager();
            ~ConnectionMagager();
            ConnectionMagager(ConnectionMagager& con) = delete;
            ConnectionMagager operator=(ConnectionMagager& con) = delete;

            void addUser(int fd, const sockaddr_in& address);
            void eraseUser(int fd);
            User& getUser(int fd);
            std::vector<pollfd>& getFds();
            void closeAll();

        private:
            std::vector<std::unique_ptr<User>> userPull_;
            std::vector<struct pollfd> fdPull_;
            std::map<int, User*> userTable_;
    };

} //namespace prx

#endif