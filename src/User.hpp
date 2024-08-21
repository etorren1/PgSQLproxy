#ifndef User_hpp
#define User_hpp

#include "Utils.hpp"

namespace prx {

    enum eStage {
        CHECK_PROTOCOL,
        CHECK_APP,
        FORWARDING
    };

    class User {

        public:
            User(int fd, int dbFd, char* ip, u_int16_t port);
            ~User();

            int getClientFd() const;
            int getDbFd() const;
            void setDbFd(int fd);
            void appendQuery(int fd, char* buf, int buf_size);
            bool isRequest(int fd) const;
            const std::vector<char>& getResponceQuery() const;
            const std::vector<char>& getRequestQuery() const;

            void clearRequest();
            void clearResponce();
            std::string getAppInfo() const;
            void setAppInfo(const std::string& app, const std::string& login, const std::string& dbname);
            
            eStage stage_ = eStage::CHECK_PROTOCOL;

        private:
        
            int userFd_ = UNKNOWN_FD;
            int dbFd_ = UNKNOWN_FD;
            std::string ipv4_;
            int port_ = 0;
            std::string appInfo_;
            std::vector<char> responceQuery_;
            std::vector<char> requestQuery_;

    };

} //namespace prx

#endif