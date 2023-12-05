#include "SocksServer.h"
#include <netdb.h>
#include <cinttypes>
#include <cstring>

#define SOCKS_VERSION 5

const char* def_address = "0.0.0.0";



bool SocksServer::authorize(int fd) {
    struct authorizeHeader {
        uint8_t version;
        uint8_t methodCount;
    } header;
    readSocket(fd, &header, sizeof(header));
    if (header.version != SOCKS_VERSION) {
        return false;
    }
    uint8_t methods[header.methodCount];
    readSocket(fd, methods, sizeof(uint8_t) * header.methodCount);
    uint8_t authMethod = 0xFF;
    for (uint8_t i = 0; i < header.methodCount; i++) {
        if (methods[i] == 0x00) {
            authMethod = 0x00;
        }
    }
    uint8_t authAnswer[2] = {SOCKS_VERSION, authMethod};
    if (authMethod == 0xFF) {
        sendSocket(fd, authAnswer, sizeof(authAnswer));
        return false;
    }
    sendSocket(fd, authAnswer, sizeof(authAnswer));
    return true;
}

bool SocksServer::readDomain(int fd) {}

bool SocksServer::acceptCommand(int fd) {
    struct {
        uint8_t version;
        uint8_t command;
        /* must be 0x00 */
        uint8_t reserved;
        uint8_t addressType;
    } header;
    readSocket(fd, &header, sizeof(header));
    if (header.version != 5)
        return -1;
    sa_family_t saFamily;
    uint8_t adrlen;
    switch (header.addressType) {
        case 0x01:
            saFamily = AF_INET;
            adrlen = 4;
        case 0x03:
            saFamily = AF_UNSPEC;
            readSocket(fd, &adrlen, sizeof(uint8_t));
        case 0x04:
            saFamily = AF_INET6;
            adrlen = 15;
            break;
        default:
            return false;
    }
    char host[adrlen];
    uint16_t port;
    readSocket(fd, host, sizeof(uint8_t) * adrlen);
    readSocket(fd, &port, sizeof(port));
    port = ntohs(port);
    if (saFamily == AF_UNSPEC) {
        char portStr[12];
        sprintf(portStr, "%" PRIu16, port);
        struct addrinfo *addrinfo;
        int err = getaddrinfo(host, portStr, nullptr, &addrinfo);
        if (err != 0) {
            return false;
        }
        std::memcpy(&targetAddress, addrinfo->ai_addr, addrinfo->ai_addrlen);
        freeaddrinfo(addrinfo);
    }
    //TODO: resolve ip addresses

}


void SocksServer::acceptor(int fd, address_t address) {
    setSocketTimeout(fd, timeout);
    if (!authorize(fd))
        return;

}

SocksServer::SocksServer(int port, struct timeval timeout) : TCPServer(def_address, port), timeout(timeout) {}

class SocksConnection {

};