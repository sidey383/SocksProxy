#pragma once

#include <string>
#include <exception>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>

namespace TCP {

    class SocketException : public std::exception {
    };

    class SocketErrnoException : public SocketException {
    private:
        int _error;
        std::string _what;
    public:
        SocketErrnoException(int error) : _error(error) {
            _what = std::string("Error: ") + strerror(_error);
        }

        const char *what() const noexcept override {
            return _what.c_str();
        }
    };

    class ConnectionException : public SocketException {
    private:
        std::string reason;
    public:
        explicit ConnectionException(std::string reason) : reason(std::move(reason)) {}

        explicit ConnectionException(const char *reason) : reason(reason) {}

        const char *what() const noexcept override {
            return reason.c_str();
        }

    };

    class AddressNotFoundException : public SocketException {
    private:
        std::string _address;
        std::string _what;
    public:
        AddressNotFoundException(const char *address) : _address(address) {
            _what = "Not found address " + _address;
        }

        AddressNotFoundException(const std::string address) : _address(address) {
            _what = "Not found address " + _address;
        }

        const char *what() const noexcept override {
            return _what.c_str();
        }
    };

    class Socket {
    protected:
        socklen_t _addrLen = 0;
        int _addrFamily = AF_UNSPEC;
        sockaddr _addr{};
        int _socket;

    public:
        Socket(int socket, socklen_t addrLen, int addrFamily, sockaddr addr) :
                _socket(socket), _addrLen(addrLen), _addrFamily(addrFamily), _addr(addr) {}

        Socket(const char *address, uint16_t port);

        void readSocket(void *buf, size_t size) const;

        void writeSocket(const void *buf, size_t size) const;

        int setTimeOut(struct timeval timeout) const;

        int removeTimeOut() const;

        void close() const;

        const char *getAddressString(char *str, size_t len);
    };

    class ClientSocket : public Socket {
    public:
        void connect();
    };

    class ServerSocket : public Socket {
    private:
        int _maxConnections;
    public:
        ServerSocket(const char *address, uint16_t port, int maxConnections) :
                Socket(address, port), _maxConnections(maxConnections) {}

        void bind();

        void listen();

        virtual void acceptor(int cliSocket, sockaddr addr) = 0;
    };
}
