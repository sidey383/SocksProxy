#pragma once
#include "../tcp/tcp.h"

class SocksServer : public TCPServer {
private:
    struct timeval timeout;

    void acceptor(int fd, address_t address) override;
public:
    SocksServer(int port, struct timeval timeout);

};
