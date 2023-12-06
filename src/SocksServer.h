#pragma once
#include "../tcp/Server.h"
#include "SocksWorker.h"

class SocksServer : public TCP::Server {
private:
    struct timeval timeout;
protected:
    void accept(TCP::Socket, struct sockaddr) override;
    static bool authorize(TCP::Socket&);
    static TCP::ipaddres acceptCommand(TCP::Socket&);
    static void sendAnswer(const TCP::Socket& socket, TCP::ipaddres);
public:
    SocksServer(uint16_t port, struct timeval timeout);

};
