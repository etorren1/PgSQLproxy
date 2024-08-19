#include "Server.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

namespace prx {

    Server::Server()
    {
        //_cfgpath.empty() ? cfgpath = DEFAULT_CFG : cfgpath = _cfgpath;
        dbInfo_.hostname = "127.0.0.1";
        dbInfo_.port = 5432;
        init();
    }

    Server::~Server()
    {
        std::cout << "Server destroyed.\n";
    }

    void    Server::init()
    {
        /* socket initialization */
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
        /* binding IP and PORT to the server socket */
        if (bind(socket_, (struct sockaddr*)&address, sizeof(address))) {
            perror("Socket bind failed");
            close(socket_);
            exit(EXIT_FAILURE);
        }
        /* start listening connections to socket */
        if (listen(socket_, 0) < 0) { //count of listeners?
            perror("listen");
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
            connectClient();
            processQuery();
            commandLine();
        }
        if (status_ & RESTART) {
            std::cout << "Restarting...\n";
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
            }
            if (text == "\\stop") {
                std::cout << "Shutdown server\n";
                close(socket_);
                cntManager_.closeAll();
                status_ = STOP;
            }
            if (text == "\\re") {
                status_ = RESTART;
            }
        }
    }

    void    Server::connectClient()
    {
        /* listen for incoming connections via poll */
        int ret = poll(&poll_, 1, 0);
        if (ret > 0) {
            if (poll_.revents & POLLIN) {
                int clientSocket;
                struct sockaddr_in clientAddr;
                int addrlen = sizeof(clientAddr);
                if ((clientSocket = accept(socket_, (struct sockaddr*)&clientAddr, (socklen_t*)&addrlen)) > 0) {
                    cntManager_.addUser(clientSocket, clientAddr);
                }
                poll_.revents = 0;
            }
        }
        /* else if (ret < 0) {
            std::cout << "poll err"  << std::endl;
        } */
    }

    void    Server::processQuery()
    {
        auto& fdPull = cntManager_.getFds();
        int ret = poll(fdPull.data(), fdPull.size(), 0);
        if (ret == -1) {
            perror("poll failed");
            exit(EXIT_FAILURE);
        }
        else if (ret != 0) {
            for (pollfd& sock : fdPull) {
                if (sock.revents & POLLIN) {
                    User& user = cntManager_.getUser(sock.fd);
                    size_t bytesRead = readQuery(user, sock.fd);

                    if (bytesRead == 0) {
                        disconnectUser(sock.fd);
                        continue;
                    }
                    if (user.isRequest(sock.fd)) {
                        handleRequest(user);
                    }
                    else {
                        handleResponce(user);
                    }
                    sock.revents = 0;
                }
            }
        }
    }

    
    static int bytesToInteger(char * buffer)
    {
        int size = static_cast<int>(static_cast<unsigned char>(buffer[0]) << 24 |
            static_cast<unsigned char>(buffer[1]) << 16 | 
            static_cast<unsigned char>(buffer[2]) << 8 | 
            static_cast<unsigned char>(buffer[3]));
        return size;
    }

    int     Server::readQuery(User& user, int fd)
    {
        char buf[BUF_SIZE + 1];
        int bytesRead = 0;
        int count;
        if ((count = recv(fd, buf, BUF_SIZE, 0)) > 0) {
            buf[count] = 0;
            if (bytesRead == 0 && count > 0) {
/*                 std::cout << "temp bytes: " << std::endl;
                for (int i = 0; i < 4; i++) {
                    printf("%d|", temp[i]);
                } */
                int size = bytesToInteger(&buf[1]);
                std::cout << "Package size: " << size << std::endl;
            }
            bytesRead += count;
            user.appendQuery(fd, buf, count);
        }

        if (bytesRead) {
            const std::vector<char>* messPtr;
            if (user.isRequest(fd)) {
                std::cout << "Scoket " << fd << " request >>" << std::endl;
                messPtr = &user.getRequestQuery();
            }
            else {
                std::cout << "Scoket " << fd << " responce >>" << std::endl;
                messPtr = &user.getResponceQuery();
            }
            std::cout << "Total bytes: " << bytesRead << std::endl;
            for (int i = 0; i < bytesRead; i++) {
                printf("%d|", (*messPtr)[i]);
            }
            printf("--------\n");
            for (int i = 0; i < bytesRead; i++) {
                if ((*messPtr)[i] > 31 && (*messPtr)[i] < 127)
                    printf("%c", (*messPtr)[i]);
                else
                    printf(".");
            }
            printf("\n--------\n");
        }
        return (bytesRead);
    }

    void    Server::handleRequest(User & user)
    {
        send(user.getDbFd(), user.getRequestQuery().data(), user.getRequestQuery().size(), 0);
        user.clearRequest();
    }

    void    Server::handleResponce(User & user)
    {
        send(user.getClientFd(), user.getResponceQuery().data(), user.getResponceQuery().size(), 0);
        user.clearResponce();
    }

    void    Server::disconnectUser(int fd)
    {
        cntManager_.eraseUser(fd);
    }

} // namespace prx