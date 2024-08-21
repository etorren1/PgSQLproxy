#ifndef Server_hpp
#define Server_hpp

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>

#include "Utils.hpp"
#include "ConnectionManager.hpp"
#include "QueryLogger.hpp"

namespace prx {

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
            void    run( void );
        
        private:

            void    init( void );

            void    connectClient();
            void    processQuery();
            void    commandLine();

            int     readQuery(User & user, int fd);
            void    handleRequest(User & user);
            void    handleResponce(User & user);

            ConnectionMagager   cntManager_;
            QueryLogger         log_;

            struct pollfd       poll_;
            int                 socket_;
            int                 status_;
            DbConnectionInfo    dbInfo_;
    };

} //namespace prx

#endif