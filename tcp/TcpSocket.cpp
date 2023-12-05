#include "TcpSocket.h"
#include <netdb.h>
#include <cstring>
#include <cinttypes>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>

namespace TCP {

    Socket::Socket(const char *address, uint16_t port) {
        struct addrinfo *addrInfoList;
        struct addrinfo hints{};
        char portStr[16];
        sprintf(portStr, "%" PRIu16, port);
        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_socktype = SOCK_STREAM;
        int err = getaddrinfo(address, portStr, &hints, &addrInfoList);
        if (err != 0) {
            throw SocketErrnoException(errno);
        }
        for (auto *i = addrInfoList; i != nullptr; i = i->ai_next) {
            if (_addrFamily == AF_INET6 && i->ai_family == AF_INET)
                continue;
            _addrFamily = i->ai_family;
            _addrLen = i->ai_addrlen;
            _addr = *(i->ai_addr);
        }
        freeaddrinfo(addrInfoList);
        _socket = socket(AF_INET, SOCK_STREAM, 0);
        if (_socket == -1) {
            throw SocketErrnoException(errno);
        }
    }

    void Socket::readSocket(void *buf, size_t size) const {
        size_t totalRead = 0;
        while (totalRead < size) {
            size_t curRead = recv(_socket, ((char *) buf) + totalRead, size - totalRead, 0);
            if (curRead == 0)
                throw ConnectionException("Connection closed");
            if (curRead == -1)
                throw SocketErrnoException(errno);
            totalRead += curRead;
        }
    }

    void Socket::writeSocket(const void *buf, size_t size) const {
        size_t totalSend = 0;
        while (totalSend < size) {
            size_t curSend = send(_socket, ((char *) buf) + totalSend, size - totalSend, 0);
            if (curSend == 0)
                throw ConnectionException("Connection closed");
            if (curSend == -1)
                throw SocketErrnoException(errno);
            totalSend += curSend;
        }
    }

    int Socket::setTimeOut(struct timeval timeout) const {
        return setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    }

    int Socket::removeTimeOut() const {
        struct timeval timeval{0, 0};
        return setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, &timeval, sizeof(timeval));
    }

    const char *Socket::getAddressString(char *str, size_t len) {
        return inet_ntop(_addrFamily, &(_addr), str, len);
    }

    void Socket::close() const {
        ::close(_socket);
    }

    void ClientSocket::connect() {
        int err = ::connect(_socket, &_addr, _addrLen);
        if (err != 0)
            throw SocketErrnoException(err);
    }

    void ServerSocket::bind() {
        int err = ::connect(_socket, &_addr, _addrLen);
        if (err != 0)
            throw SocketErrnoException(err);
    }

    void ServerSocket::listen() {
        int err;
        sockaddr cliAddr{};
        while (true) {
            std::cout << "[" << getpid() << "] Start Listen\n";
            if ((err = ::listen(_socket, _maxConnections)) < 0) {
                std::cerr << "[" << getpid() << "] Listen error: " << strerror(err) << "\n";
                return;
            }
            int fd;
            if ((fd = ::accept(_socket, &cliAddr, &_addrLen)) < 0) {
                std::cerr << "[" << getpid() << "] Connect error: " << strerror(fd) << "\n";
            } else {
                int pid = fork();
                if (pid < 0)
                    std::cerr << "[" << getpid() << "] Fork fail: " << strerror(pid) << "\n";
                if (pid == 0) {
                    char str[128];
                    inet_ntop(_addrFamily, &(_addr), str, 128);
                    std::cout << "[" << getpid() << "] Connect from: " << str << "\n";
                    try {
                        acceptor(fd, cliAddr);
                    } catch (std::exception &ex) {
                        std::cerr << "[" << getpid() << "] Connection accept error: " << ex.what() << "\n";
                    }
                    return;
                }
            }
        }
    }
}