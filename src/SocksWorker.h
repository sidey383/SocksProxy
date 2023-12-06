#pragma one
#include "../tcp/Socket.h"

#define SOCKS_BUFFER_SIZE 8192
class SocksWorker {
private:
    TCP::Socket _client;
    TCP::Socket _server;
    char _buffer[SOCKS_BUFFER_SIZE];
protected:
    // POLLIN | POLLPRI
    bool proxyInPacket(int in, int out);
    // POLLHUP
    bool proxyError(int in, int out);
    // POLLHUP
    bool proxyClose(int in, int out);
    // POLLNVAL
    bool proxyWrongWal(int in, int out);
public:
    SocksWorker(TCP::Socket client, TCP::Socket server) : _client(client), _server(server) {}
    ~SocksWorker();
    void work();
};
