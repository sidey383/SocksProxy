#include "Server.h"
#include <unistd.h>
#include <iostream>

namespace TCP {
    Server::Server(const char *address, uint16_t port) : _socket(TCP::bindSocket(address, port)) {}

    void Server::listen() {
        int err;
        sockaddr cliAddr{};
        socklen_t socklen = _socket.addressLen();
        while (true) {
            err = ::listen(_socket.socket(), 10);
            if (err != 0)
                throw ConnectionErrnoException();
            int fd = ::accept(_socket.socket(), &cliAddr, &socklen);
            if (fd > 0) {
                int pid = ::fork();
                if (pid < 0)
                    throw ConnectionErrnoException();
                if (pid == 0) {
                    Socket socket(fd, cliAddr, _socket.addressLen());
                    try {
                        accept(socket, cliAddr);
                    } catch(std::exception& e) {
                        std::cerr << "Connection exception " << e.what() << "\n";
                        socket.close();
                    }
                    return;
                }
            } else {
                throw ConnectionErrnoException();
            }
        }
    }
}