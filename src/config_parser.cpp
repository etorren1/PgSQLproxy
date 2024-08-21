#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "Utils.hpp"

#define DEFAULT_CFG "proxy.conf"

static void get_attribute(std::string& dest, const std::string & text, const std::string & key ) {
    size_t pos = text.find(key);
    if (pos == std::string::npos)
        return;
    if ((pos && !isspace(text[pos - 1])) || !isspace(text[pos + key.size()]))
        return;
    size_t end = text.find("\n", pos);
    dest = utils::trim(text.substr(pos, end - pos).erase(0, key.size()), " \n\t\r");
}

static int     parse_config(const char* path, struct config* cfg)
{
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        return (EXIT_FAILURE);
    }
    char buf[1024];
    int rd;
    std::string text;
    while ((rd = read(fd, buf, 1023)) > 0) {
        buf[rd] = 0;
        text += buf;
    } // read config file
    get_attribute(cfg->srvhost, text, "SERVERHOST");
    get_attribute(cfg->srvport, text, "SERVERPORT");
    get_attribute(cfg->dbhost, text, "DBHOST");
    get_attribute(cfg->dbport, text, "DBPORT");
    get_attribute(cfg->logname, text, "LOG_FILE");
    get_attribute(cfg->logpath, text, "LOG_PATH");
    if (cfg->srvhost == "localhost")
        cfg->srvhost = "127.0.0.1";
    if (cfg->dbhost == "localhost")
        cfg->srvhost = "127.0.0.1";
    return 0;
}

void show_help(const char* appname);

int     parse_args(int argc, char **argv, struct config* cfg)
{
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
                show_help(argv[0]);
                return (EXIT_FAILURE);
            }
            else if (!strcmp(argv[i], "--config") || !strcmp(argv[i], "-c")) {
                if (i + 1 != argc && argv[i + 1][0] != '-') {
                    int rd = parse_config(argv[++i], cfg);
                    if (rd == EXIT_FAILURE) {
                        std::cout << argv[0] << ": No such file: " << argv[i] << "\n";
                        return (EXIT_FAILURE);
                    }
                    return (EXIT_SUCCESS);
                }
                else {
                    std::cout << argv[0] << ": Unable to get flag parameter '" << argv[i] << "'\nTry '" << argv[0] << " --help' for more information\n";
                    return (EXIT_FAILURE);
                }
            }
            else if (!strcmp(argv[i], "--server-host") || !strcmp(argv[i], "-sh")) {
                if (i + 1 != argc && argv[i + 1][0] != '-') 
                    cfg->srvhost = argv[++i];
                else {
                    std::cout << argv[0] << ": Unable to get flag parameter '" << argv[i] << "'\nTry '" << argv[0] << " for more information\n";
                    return (EXIT_FAILURE);
                }
            }
            else if (!strcmp(argv[i], "--server-port") || !strcmp(argv[i], "-sp")) {
                if (i + 1 != argc && argv[i + 1][0] != '-')
                    cfg->srvport = argv[++i];
                else {
                    std::cout << argv[0] << ": Unable to get flag parameter '" << argv[i] << "'\nTry '" << argv[0] << " for more information\n";
                    return (EXIT_FAILURE);
                }
            }
            else if (!strcmp(argv[i], "--database-host") || !strcmp(argv[i], "-dh")) {
                if (i + 1 != argc && argv[i + 1][0] != '-')
                    cfg->dbhost = argv[++i];
                else {
                    std::cout << argv[0] << ": Unable to get flag parameter '" << argv[i] << "'\nTry '" << argv[0] << " for more information\n";
                    return (EXIT_FAILURE);
                }
            }
            else if (!strcmp(argv[i], "--database-port") || !strcmp(argv[i], "-dp")) {
                if (i + 1 != argc && argv[i + 1][0] != '-')
                    cfg->dbport = argv[++i];
                else {
                    std::cout << argv[0] << ": Unable to get flag parameter '" << argv[i] << "'\nTry '" << argv[0] << " for more information\n";
                    return (EXIT_FAILURE);
                }
            }
        }
        if (cfg->srvhost == "localhost")
            cfg->srvhost = "127.0.0.1";
        if (cfg->dbhost == "localhost")
            cfg->srvhost = "127.0.0.1";
    }
    else {
        parse_config(DEFAULT_CFG, cfg);
    }
    return (EXIT_SUCCESS);
}

void show_help(const char* appname)
{
    std::cout << "Usage:\n "
        << appname << " [OPTION] [VALUE] ...\n\n"
        << "General options:\n"
        << " -c, --config           path to config file\n"
        << "if path to config file specified other options will be ignored\n"
        << " -sh, --server-host     proxy server host (default: \"127.0.0.1\")\n"
        << " -sp, --server-port     proxy server port (default: \"4242\")\n"
        << " -dh, --database-host   database server host (default: \"127.0.0.1\")\n"
        << " -dp, --database-port   database server port (default: \"5432\")\n";
}