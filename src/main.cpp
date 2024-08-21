#include "Server.hpp"

int    parse_args(int argc, char **argv, struct config * cfg);

int     main(int argc, char **argv)
{
    struct config cfg;
    if (parse_args(argc, argv, &cfg))
        return (EXIT_FAILURE);

    prx::Server server(cfg.srvhost, std::stoi(cfg.srvport),
                        cfg.dbhost, std::stoi(cfg.dbport),
                        cfg.logname, cfg.logpath);
    server.run();
    return (0);
}
