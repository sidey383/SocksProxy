#include "SocksServer.h"
#include <iostream>



int main(int argc, char **argv) {
    if (argc < 2)
        return 0;
    auto port = (uint16_t) strtol(argv[1], nullptr, 10);
    try {
        SocksServer server(port, {1, 0});
        server.listen();
        server.socket().close();
    } catch (std::exception& e) {
        std::cerr << e.what();
    }

}
