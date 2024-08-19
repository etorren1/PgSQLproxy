#ifndef User_hpp
#define User_hpp

#include <string>
#include <iostream>
#include <vector>

namespace prx {

    class User {
        public:
            User(int fd, int dbFd, char* ip);
            ~User();

            int getClientFd() const;
            int getDbFd() const;
            void appendQuery(int fd, char* buf, int buf_size);
            bool isRequest(int fd) const;
            const std::vector<char>& getResponceQuery();
            const std::vector<char>& getRequestQuery();

            void clearRequest();
            void clearResponce();
            
        private:
        
            int dbFd_ = -1;
            int userFd_ = -1;
            std::string ipv4_;
            std::vector<char> responceQuery_;
            std::vector<char> requestQuery_;

    };

} //namespace prx

#endif