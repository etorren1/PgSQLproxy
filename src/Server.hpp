#ifndef Server_hpp
#define Server_hpp

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>
#include <vector>

#include "ConnectionManager.hpp"
#include "QueryLogger.hpp"

namespace prx {

    /* Postgres constants */
    #define PqMsg_Query         'Q'
    #define PG_PROTOCOL(m,n)    (((m) << 16) | (n))
    #define NEGOTIATE_SSL_CODE PG_PROTOCOL(1234,5679)
    /* End */

    #define BUF_SIZE 8192 // Max count of readed bytes per recv()

    #define STOP        0x00
    #define RESTART     0x01
    #define WORKING     0x02

    //#define DEFAULT_CFG "proxy.conf"

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
            void    disconnectUser(User& user);

            ConnectionMagager   cntManager_;
            QueryLogger         log_;

            struct pollfd       poll_;
            int                 socket_;
            int                 status_;
            DbConnectionInfo    dbInfo_;
    };

} //namespace prx

#endif