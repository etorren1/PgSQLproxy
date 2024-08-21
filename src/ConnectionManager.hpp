#ifndef ConnectionMagager_hpp
#define ConnectionMagager_hpp

#include <netinet/in.h>
#include <poll.h>

#include "Utils.hpp"
#include "User.hpp"

namespace prx {

    class ConnectionMagager {

        public:
            ConnectionMagager();
            ~ConnectionMagager();
            ConnectionMagager(ConnectionMagager& con) = delete;
            ConnectionMagager operator=(ConnectionMagager& con) = delete;

            bool    checkDbConnection();
            void    addUser(int fd, const sockaddr_in& address);
            void    removeUser(User& user);
            User&   getUser(int fd);
            void    closeAll();
            std::vector<pollfd>& getFds();

        private:
            int     createDbSocket(); //! Create new database socket. Return socket ID or -1 if error occured.

            std::vector<std::unique_ptr<User>> userPull_;   //! Pull of all users with two sockets
            std::vector<struct pollfd> fdPull_;             //! Pull of all sockets for listening via poll
            std::map<int, User*> userTable_;                //! Simple hash table for quic find user by socket 
    };

} //namespace prx

#endif