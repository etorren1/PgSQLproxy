#ifndef Server_hpp
#define Server_hpp

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>
#include <vector>

#include "ConnectionManager.hpp"

namespace prx {

    #define STOP    0
    #define WORKING 0b10
    #define RESTART 0b01
    #define BUF_SIZE 1024
    //#define DEFAULT_CFG "proxy.conf"

    class User;

    struct DbConnectionInfo {
        std::string hostname;
        int port;
    } ;

    class Server {
        
        public:

            Server( void );
            ~Server();

            Server  operator=(const Server & src) = delete; 

            //void    config( const int & fd );
            void    init( void );
            void    run( void );
        
        private:

            void    connectClient();
            void    processQuery();
            void    commandLine();

            int     readQuery(User & user, int fd);
            void    handleRequest(User & user);
            void    handleResponce(User & user);
            void    disconnectUser(int fd);

            ConnectionMagager   cntManager_;

            struct pollfd       poll_;
            int                 socket_;
            int                 status_;
            DbConnectionInfo    dbInfo_;
    };

} //namespace prx

#endif