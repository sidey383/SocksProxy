#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include "tcp.h"

#define MAX_CONNECTION 10

TCPServer::~TCPServer() {
    if (fdS > 0)
        close(fdS);
}

void TCPServer::bindAddress() {
    fdS = socket(AF_INET, SOCK_STREAM, 0);
    if (fdS == -1) {
        throw TCPError("socket creation failed");
    }
    if (address.ip.sa_family == AF_INET) {
        if (bind(fdS, static_cast<struct sockaddr *>(&address.ip), sizeof(struct sockaddr_in)) != 0) {
            throw TCPError(strerror(errno));
        }
        return;
    }
    if (address.ip.sa_family == AF_INET6) {
        if (bind(fdS, static_cast<struct sockaddr *>(&address.ip), sizeof(struct sockaddr_in6)) != 0) {
            throw TCPError(strerror(errno));
        }
        return;
    }
}

void TCPServer::startLister() {
    int err;
    socklen_t len = address.ip.sa_family == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
    address_t cliAddr;
    cliAddr.ip.sa_family = address.ip.sa_family;
    while (true) {
        std::cout << "[" << getpid() << "] tart Listen\n";
        if ((err = listen(fdS, MAX_CONNECTION)) < 0) {
            std::cerr << "[" << getpid() << "] Listen error: " << strerror(err) << "\n";
            return;
        }
        int fd;
        if ((fd = accept(fdS, &cliAddr.ip, &len)) < 0) {
            std::cerr << "[" << getpid() << "] Connect error: " << strerror(fd) << "\n";
        } else {
            int pid = fork();
            if (pid < 0)
                std::cerr << "[" << getpid() << "] Fork fail: " << strerror(pid) << "\n";
            if (pid == 0) {
                std::cout << "[" << getpid() << "] Connect from: " << toString(cliAddr) << "\n";
                try {
                    acceptor(fd, cliAddr);
                } catch (ConnectionException &conEx) {
                    std::cerr << "[" << getpid() << "] Connection error: " << conEx.what() << "\n";
                } catch (std::exception &ex) {
                    std::cerr << "[" << getpid() << "] Unknown exception: " << ex.what() << "\n";
                }
                close(fd);
                std::cout << "[" << getpid() << "] Close connect from: " << toString(cliAddr) << "\n";
                return;
            }
        }
    }
}

