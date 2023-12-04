#include "SocksServer.h"
#include <iostream>
#include <unistd.h>

const char* def_address = "0.0.0.0";

typedef struct {
    uint8_t version;
    uint8_t command;
    /* must be 0x00 */
    uint8_t reserved;
    uint8_t addressType;
} clientRequestHeader;


void SocksServer::acceptor(int fd, address_t address) {
    std::cout << "[" << getpid()
              << "] Accept connection from " << toString(address)
              << "\n";
    setSocketTimeout(fd, timeout);
    uint8_t version;
    readSocket(fd, &version, sizeof(uint8_t));
    if (version != 5) {
        return;
    }
    uint8_t methodCount;
    readSocket(fd, &methodCount, sizeof(uint8_t));
    uint8_t methods[methodCount];
    readSocket(fd, methods, sizeof(uint8_t) * methodCount);
    uint8_t authMethod = 0xFF;
    for (uint8_t i = 0; i < methodCount; i++) {
        if (methods[i] == 0x00) {
            authMethod = 0x00;
        }
    }
    uint8_t authAnswer[2] = {5, authMethod};
    sendSocket(fd, authAnswer, sizeof(authAnswer));
    if (authMethod == 0xFF)
        return;
    clientRequestHeader header;
    readSocket(fd, &header, sizeof(header));
    if (header.version != 5)
        return;
}

SocksServer::SocksServer(int port, struct timeval timeout) : TCPServer(def_address, port), timeout(timeout) {}

class SocksConnection {

};