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

    class Server {
        
        public:

            Server(const std::string& srvhost, int srvport,
                   const std::string& dbhost, int dbport,
                   const std::string& logname, const std::string& logpath);
            ~Server();

            Server( void ) = delete;
            Server(Server& s) = delete;
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

            QueryLogger         log_;
            ConnectionMagager   cntManager_;

            struct pollfd       poll_;
            int                 socket_;
            int                 status_;
            ConnectionInfo      srvInfo_;

    };

} //namespace prx

#endif