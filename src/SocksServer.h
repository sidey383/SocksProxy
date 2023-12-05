#pragma once
#include "../tcp/tcp.h"

class SocksServer : public TCPServer {
private:
    struct timeval timeout;
    address_t targetAddress{};

    void acceptor(int fd, address_t address) override;
    bool authorize(int fd);
    bool acceptCommand(int fd);
    bool readIpV4(int fd);
    bool readIpV6(int fd);
    bool readDomain(int fd);
public:
    SocksServer(int port, struct timeval timeout);

};
