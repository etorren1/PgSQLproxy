#include "Server.hpp"

int     main(int argc, char **argv)
{
/*     int fd;
    std::string cfgpath;
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " (config_file)\n";
        return (1);
    }
    else if (argc == 2) {
        fd = open(argv[1], O_RDONLY);
        cfgpath = argv[1];
        if (fd == -1) {
            std::cerr << argv[0] << ": no such file: " << argv[1] << "\n";
            return (1);
        }
    }
    else {
        fd = open(DEFAULT_CFG, O_RDONLY);
        cfgpath.clear();
        if (fd == -1) {
            std::cerr << "proxy: can not find proxy.conf in the standard path\n"
            << "specify the path to the config file: " << argv[0] << " (path/config_file)" "\n";
            return (1);
        }
    }*/

    prx::Server server;
    //server.config(fd);
    server.run();


    return (0);
}