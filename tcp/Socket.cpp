#include "Socket.h"
#include <netdb.h>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <sstream>
#include <string>
#include <iostream>

namespace TCP {

    ConnectionErrnoException::ConnectionErrnoException(int error) : _error(error) {
        _strerror = ::strerror(error);
        size_t errLen = strlen(_strerror);
        _what = (char *) malloc(prefixLen + errLen + sizeof(char));
        ::memcpy(_what, prefix, prefixLen);
        ::memcpy(_what + prefixLen, _strerror, errLen);
        _what[prefixLen + errLen] = 0;
    }

    ConnectionErrnoException::ConnectionErrnoException() : ConnectionErrnoException(errno) {}

    ConnectionErrnoException::~ConnectionErrnoException() {
        free(_what);
    }

    const char *SocketConnectionClosedException::what() const noexcept {
        return "Connection closed";
    }

    Socket::Socket(TCP::Socket &s) = default;

    void Socket::close() const {
        ::close(_socket);
    }

    void Socket::readSocket(void *buf, size_t size) const {
        size_t totalRead = 0;
        while (totalRead < size) {
            size_t curRead = recv(_socket, ((char *) buf) + totalRead, size - totalRead, 0);
            if (curRead == 0)
                throw SocketConnectionClosedException();
            if (curRead == -1)
                throw ConnectionErrnoException();
            totalRead += curRead;
        }
    }

    void Socket::writeSocket(const void *buf, size_t size) const {
        size_t totalSend = 0;
        while (totalSend < size) {
            size_t curSend = send(_socket, ((char *) buf) + totalSend, size - totalSend, 0);
            if (curSend == 0)
                throw SocketConnectionClosedException();
            if (curSend == -1)
                throw ConnectionErrnoException();
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

    struct ipaddres selectAddr(const char *address, uint16_t port) {
        struct ipaddres result{};
        struct addrinfo *addrInfoList;
        struct addrinfo hints{};
        char portStr[16];
        sprintf(portStr, "%" PRIu16, port);
        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_socktype = SOCK_STREAM;
        int err = getaddrinfo(address, portStr, &hints, &addrInfoList);
        if (err != 0)
            throw ConnectionErrnoException(err);
        for (auto *i = addrInfoList; i != nullptr; i = i->ai_next) {
            if (i->ai_family != AF_INET6 && i->ai_family != AF_INET)
                continue;
            if (result.family == AF_INET && i->ai_family == AF_INET6)
                continue;
            result.family = i->ai_family;
            result.addr = *i->ai_addr;
            result.addrlen = i->ai_addrlen;
        }
        if (result.family == AF_INET)
            ((struct sockaddr_in*) &result.addr)->sin_port = htons(port);
        if (result.family == AF_INET6)
            ((struct sockaddr_in6*) &result.addr)->sin6_port = htons(port);
        return result;
    }

    TCP::Socket bindSocket(const char *address, uint16_t port) {
        return bindSocket(selectAddr(address, port));
    }

    Socket bindSocket(const ipaddres& address) {
        int sfd = socket(address.family, SOCK_STREAM, 0);
        if (sfd == -1)
            throw ConnectionErrnoException();
        int err = ::bind(sfd, &address.addr, address.addrlen);
        if (err != 0)
            throw ConnectionErrnoException();
        return TCP::Socket(sfd, address.addr, address.addrlen);
    }

    TCP::Socket connectSocket(const char *address, uint16_t port) {
        return connectSocket(selectAddr(address, port));
    }

    Socket connectSocket(const ipaddres& address) {
        int sfd = socket(address.family, SOCK_STREAM, 0);
        if (sfd == -1)
            throw ConnectionErrnoException();
        int err = ::connect(sfd, &address.addr, address.addrlen);
        if (err != 0)
            throw ConnectionErrnoException();
        return TCP::Socket(sfd, address.addr, address.addrlen);
    }

    std::string toString(const sockaddr& address) {
        char buf[128];
        std::stringstream ss;
        if (address.sa_family == AF_INET6) {
            if (inet_ntop(AF_INET6, &(((sockaddr_in6*)&address)->sin6_addr), buf, 128) == nullptr)
                return "";
            ss << "[" << buf << "]:" << ntohs(((sockaddr_in6*)&(address))->sin6_port);
        }
        if (address.sa_family == AF_INET) {
            if (inet_ntop(AF_INET, &(((sockaddr_in*)&address)->sin_addr), buf, 128) == nullptr)
                return "";
            ss << buf << ":" << ntohs(((const sockaddr_in*)&(address))->sin_port);
        }
        return ss.str();
    }

}