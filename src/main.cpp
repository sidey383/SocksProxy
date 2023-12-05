#include <iostream>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring>
#include <cinttypes>


int main(int argc, char **argv) {
    if (argc < 2)
        return 0;
    uint16_t port = 10;
    char portStr[12];
    sprintf(portStr, "%" PRIu16, port);
    std::cout << portStr << "\n";
    struct addrinfo *addrinfo;
    int err = getaddrinfo(argv[1], argc < 3 ? nullptr : argv[2], nullptr, &addrinfo);
    if (err != 0) {
        std::cerr << "getaddrinfo error: " << strerror(err);
        return -1;
    }
    char ip[128];
    for (struct addrinfo *info = addrinfo; info != nullptr; info = info->ai_next) {
        struct sockaddr *addr = info->ai_addr;
        if ( info->ai_canonname != nullptr)
            std::cout << "canonname " << info->ai_canonname;
        std::cout << " family " << info->ai_family
                  << " flags " << info->ai_flags
                  << " socktype " << info->ai_socktype
                  << " ip ";
        if (addr->sa_family == AF_INET) {
            auto *addrin = (struct sockaddr_in *) addr;
            if (inet_ntop(addrin->sin_family, &(addrin->sin_addr), ip, 128) != nullptr) {
                std::cout << ip << "\n";
            } else {
                std::cerr << "iner_ntop error: " << strerror(errno) << "\n";
            }
        }
        if (addr->sa_family == AF_INET6) {
            auto *addrin = (struct sockaddr_in6 *) addr;
            if (inet_ntop(addrin->sin6_family, &(addrin->sin6_addr), ip, 128) != nullptr) {
                std::cout << ip << "\n";
            } else {
                std::cerr << "iner_ntop error: " << strerror(errno) << "\n";
            }
        }
    }
    return 0;
}
