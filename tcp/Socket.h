#pragma once

#include <cstdlib>
#include <exception>
#include <cstring>
#include <cinttypes>
#include <arpa/inet.h>
#include <cerrno>
#include <string>

namespace TCP {

    struct ipaddres{
        socklen_t addrlen;
        int family;
        struct sockaddr addr;
    };

    class ConnectionException : public std::exception {
    };

    class ConnectionErrnoException : public ConnectionException {
    private:
        constexpr static char prefix[] = "Error: ";
        constexpr static size_t prefixLen = sizeof(prefix) - sizeof(char);
    protected:
        int _error;
        char *_what;
        char *_strerror;
    public:
        explicit ConnectionErrnoException(int error);

        ConnectionErrnoException();

        ~ConnectionErrnoException() override;

        [[nodiscard]] const char *what() const noexcept override {
            return _what;
        }

        [[nodiscard]] const char *strerror() const noexcept {
            return _strerror;
        }

        [[nodiscard]] int error() const noexcept {
            return _error;
        }
    };

    class SocketConnectionClosedException : public ConnectionException {
        [[nodiscard]] const char * what() const noexcept override;
    };

    class Socket {
    private:
        const int _socket;
        const sockaddr _addr;
        const socklen_t _addrLen;
    public:

        explicit Socket(const int socket, const sockaddr& addr, const socklen_t& addrLen) : _socket(socket), _addr(addr), _addrLen(addrLen) {}

        Socket(Socket &);

        Socket() : _socket(0), _addrLen(0), _addr({}) {}

        void readSocket(void *buf, size_t size) const;

        void writeSocket(const void *buf, size_t size) const;

        [[nodiscard]] int setTimeOut(struct timeval timeout) const;

        [[nodiscard]] int removeTimeOut() const;

        [[nodiscard]] int socket() const {
            return _socket;
        }

        [[nodiscard]] sockaddr address() const {
            return _addr;
        }

        [[nodiscard]] socklen_t addressLen() const {
            return _addrLen;
        }

        void close() const;
    };

    struct ipaddres selectAddr(const char *address, uint16_t port);

    Socket bindSocket(const char *address, uint16_t port);

    Socket bindSocket(const ipaddres& address);

    Socket connectSocket(const char *address, uint16_t port);

    Socket connectSocket(const ipaddres& address);

    std::string toString(const sockaddr& address);

}
