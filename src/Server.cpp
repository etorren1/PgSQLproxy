#include "Server.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

namespace prx {

    Server::Server() : log_("Proxy", "logs")
    {
        //_cfgpath.empty() ? cfgpath = DEFAULT_CFG : cfgpath = _cfgpath;
        dbInfo_.hostname = "127.0.0.1";
        dbInfo_.port = 5432;
        init();
    }

    Server::~Server()
    {
    }

    void    Server::init()
    {
        /* Socket initialization */
        if ((socket_ = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Socket initialization failed");
            close(socket_);
            exit(EXIT_FAILURE);
        }
        struct sockaddr_in  address;
        address.sin_family = AF_INET;
        address.sin_port = htons(std::stoi("4242"));
        if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) < 0)
        {
            perror("Unable IP translation to special numeric format");
            close(socket_);
            exit(EXIT_FAILURE);
        }
        /* Binding IP and PORT to the server socket */
        if (bind(socket_, (struct sockaddr*)&address, sizeof(address))) {
            perror("Socket bind failed");
            close(socket_);
            exit(EXIT_FAILURE);
        }
        /* Start listening connections to socket */
        if (listen(socket_, BACKLOG) < 0) {
            perror("Socket listen failed");
            close(socket_);
            exit(EXIT_FAILURE);
        }

        if (!cntManager_.checkDbConnection()) {
            perror("Connection to database refused");
            close(socket_);
            exit(EXIT_FAILURE);
        }

        poll_.fd = socket_;
        poll_.events = POLLIN;
        poll_.revents = 0;

        /* Set socket nonblock for async listening */
        fcntl(socket_, F_SETFL, O_NONBLOCK);
        fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
    }

    void    Server::run()
    {
        std::cout << "Server is now running. Type \"help\" for help.\n";
        status_ = WORKING;
        while(status_ & WORKING) {
            try {
                connectClient();
                processQuery();
                commandLine();
            }
            catch (std::exception& e) {
                cntManager_.closeAll();
                std::cerr << "Error: " << e.what() << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        if (status_ & RESTART) {
            cntManager_.closeAll();
            run();
        }
    }

    void    Server::commandLine()
    {
        char buf[BUF_SIZE + 1];
        int bytesRead = 0;
        std::string text;
        bytesRead = read(fileno(stdin), buf, BUF_SIZE);
        buf[bytesRead - 1] = 0; // replace '\n' as end of string
        text.assign(buf);
        if (bytesRead > 0) {
            if (text == "help") {
                std::cout << "Type: \\stop for shutdown server\n";
                std::cout << "      \\re for restarting server\n";
                std::cout << "      \\nl for start new log file\n";
            }
            else if (text == "\\stop") {
                std::cout << "Shutdown server\n";
                close(socket_);
                cntManager_.closeAll();
                status_ = STOP;
            }
            else if (text == "\\re") {
                std::cout << "Restarting server\n";
                status_ = RESTART;
            }
            else if (text == "\\nl") {
                if (log_.newLog())
                    std::cout << "New log file created\n";
            }
        }
    }

    void    Server::connectClient()
    {
        /* Listen for incoming connections via poll */
        int ret = poll(&poll_, 1, 100);
        if (ret > 0) {
            if (poll_.revents & POLLIN) {
                int clientSocket;
                struct sockaddr_in clientAddr;
                int addrlen = sizeof(clientAddr);
                if ((clientSocket = accept(socket_, (struct sockaddr*)&clientAddr, (socklen_t*)&addrlen)) > 0) {
                    cntManager_.addUser(clientSocket, clientAddr);
                }
                else if (clientSocket == ERROR) {
                    perror("Accept new connection failed");
                }
                poll_.revents = 0;
            }
        }
        else if (ret == ERROR) {
            throw std::runtime_error("Poll error");
        }
    }

    void    Server::processQuery()
    {
        auto& fdPull = cntManager_.getFds();
        int ret = poll(fdPull.data(), fdPull.size(), 0);
        if (ret == ERROR) {
            throw std::runtime_error("Poll error");
        }
        else if (ret != 0) {
            std::vector<User*> disconnectQueue;
            for (pollfd& sock : fdPull) {
                if (sock.revents & POLLIN) {
                    User& user = cntManager_.getUser(sock.fd);
                    bool isRequest = user.isRequest(sock.fd);
                    size_t bytesRead = readQuery(user, sock.fd);

                    if (bytesRead == 0 && isRequest) {
                        disconnectQueue.push_back(&user);
                        continue;
                    }
                    if (isRequest) {
                        handleRequest(user);
                    }
                    else {
                        handleResponce(user);
                    }
                    sock.revents = 0;
                }
            }
            for (User* user : disconnectQueue) {
                cntManager_.removeUser(*user);
            }
        }
    }

    int     Server::readQuery(User& user, int fd)
    {
        char buf[BUF_SIZE + 1];
        int bytesRead = 0;
        int count;
        if ((count = recv(fd, buf, BUF_SIZE, 0)) > 0) {
            buf[count] = 0;
            bytesRead += count;
            user.appendQuery(fd, buf, count);
        }
        return (bytesRead);
    }

    void    Server::handleRequest(User & user)
    {
        auto& query = user.getRequestQuery();
        int querySize = query.size();

        const int MSG_TYPE_SIZE = 1;    // query type size
        const int DELIM_SIZE  = 1;      // key-value delimeter size
        const int BYTE_COUNT_SIZE = 4;  // count bytes of query size

        switch (user.stage_)
        {
        case eStage::FORWARDING: {
            if (querySize >= BYTE_COUNT_SIZE + MSG_TYPE_SIZE) {
                int totalPackageSize = MSG_TYPE_SIZE + utils::bytesToInteger(query.data() + MSG_TYPE_SIZE); // shifted by MSG_TYPE_SIZE because the message type is not included in the total length
                if (totalPackageSize == querySize) {
                    if (query[0] == PqMsg_Query) {
                        std::string text = query.data() + MSG_TYPE_SIZE + BYTE_COUNT_SIZE;
                        log_.writeLog(text, user.getAppInfo());
                    }
                }
                else {
                    return; // wait more bytes
                }
            }
            else {
                return; // wait more bytes
            }
            break;
        }
        case eStage::CHECK_APP: {
            if (querySize >= BYTE_COUNT_SIZE) {
                int totalPackageSize = utils::bytesToInteger(query.data());
                if (totalPackageSize == querySize) {
                    const char *it = query.data() + BYTE_COUNT_SIZE;
                    if (querySize >= 8 && it[0] == 0 && it[1] == 3 && it[2] == 0 && it[3] == 0) {
                        //std::cout << "Protocol 3.0 enabed\n";
                    }

                    it += BYTE_COUNT_SIZE;
                    std::string login, app, dbname;
                    while (*it != '\0') {
                        std::string key = it;
                        std::string value = it + key.size() + DELIM_SIZE;
                        if (key == "user") {
                            login = value;
                        }
                        else if (key == "database") {
                            dbname = value;
                        }
                        else if (key == "application_name") {
                            app = value;
                        }
                        it += key.size() + DELIM_SIZE + value.size() + DELIM_SIZE;
                    }
                    user.setAppInfo(app, login, dbname);
                    user.stage_ = eStage::FORWARDING;
                }
                else {
                    return; // wait more bytes
                }
                
            }
            else {
                return; // wait more bytes
            }
            break;
        }
        case eStage::CHECK_PROTOCOL: {
            if (querySize >= 8) {
                /* Parse first package */
                if (NEGOTIATE_SSL_CODE == utils::bytesToInteger(query.data() + BYTE_COUNT_SIZE)) {
                    user.stage_ = eStage::CHECK_APP;
                }
                else {
                    std::string message = "Unsupported protocol";
                    send(user.getClientFd(), message.c_str(), message.size(), 0);
                    cntManager_.removeUser(user);
                    return;
                }
            }
            else {
                return; // wait more bytes
            }
            break;
        }
        default:
            break;
        }

        send(user.getDbFd(), query.data(), querySize, 0);
        user.clearRequest();
    }

    void    Server::handleResponce(User & user)
    {
        send(user.getClientFd(), user.getResponceQuery().data(), user.getResponceQuery().size(), 0);
        user.clearResponce();
    }

} // namespace prx