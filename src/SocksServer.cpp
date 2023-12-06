#include "SocksServer.h"
#include <netdb.h>
#include <iostream>

#define SOCKS_VERSION 5

const char* def_address = "127.0.0.1";



bool SocksServer::authorize(TCP::Socket& socket) {
    struct authorizeHeader {
        uint8_t version;
        uint8_t methodCount;
    } header{};
    socket.readSocket(&header, sizeof(header));
    if (header.version != SOCKS_VERSION) {
        std::cerr << "Wrong protocol version! " << (int) header.version << "\n";
        return false;
    }
    uint8_t methods[header.methodCount];
    socket.readSocket(methods, sizeof(uint8_t) * header.methodCount);
    uint8_t authMethod = 0xFF;
    for (uint8_t i = 0; i < header.methodCount; i++) {
        if (methods[i] == 0x00) {
            authMethod = 0x00;
        }
    }
    uint8_t authAnswer[2] = {SOCKS_VERSION, authMethod};
    if (authMethod == 0xFF) {
        socket.writeSocket(authAnswer, sizeof(authAnswer));
        std::cerr << "Can't found auth method!\n";
        return false;
    }
    socket.writeSocket(authAnswer, sizeof(authAnswer));
    return true;
}

void sendRequestAnswer(TCP::Socket& socket, void* header, void* address, size_t addressSize, uint16_t port) {
    char packet[4 + addressSize + 2];
    ::memcpy(packet, header, 4);
    ::memcpy(packet + 4, address, addressSize);
    port = ntohs(port);
    ::memcpy(packet + 4 + addressSize, &port, 2);
    socket.writeSocket(packet, 4 + addressSize + 2);
}

TCP::ipaddres SocksServer::acceptCommand(TCP::Socket& client) {
    TCP::ipaddres result{};
    struct {
        uint8_t version;
        uint8_t command;
        /* must be 0x00 */
        uint8_t reserved;
        uint8_t addressType;
    } header{};
    sa_family_t saFamily;
    uint8_t addrLen;
    client.readSocket(&header, sizeof(header));
    if (header.version != SOCKS_VERSION) {
        std::cerr << "Wrong protocol version! " << header.version << "\n";
        return result;
    }
    switch (header.addressType) {
        case 0x01:
            saFamily = AF_INET;
            addrLen = 4;
            break;
        case 0x03:
            saFamily = AF_UNSPEC;
            client.readSocket(&addrLen, sizeof(uint8_t));
            break;
        case 0x04:
            saFamily = AF_INET6;
            addrLen = 16;
            break;
        default:
            std::cerr << "Wrong address type!\n";
            return result;
    }
    char addrByte[addrLen + 1];
    uint16_t port;
    client.readSocket(addrByte, sizeof(uint8_t) * addrLen);
    addrByte[addrLen] = 0;
    client.readSocket(&port, sizeof(port));
    port = ntohs(port);
    if (saFamily == AF_INET) {
        auto* addr4 = (sockaddr_in*) &result.addr;
        addr4->sin_port = 0;
        ::memcpy(&addr4->sin_addr, addrByte, addrLen);
        addr4->sin_family = AF_INET;
    }
    if (saFamily == AF_INET6) {
        auto* addr6 = (sockaddr_in6*) &result.addr;
        addr6->sin6_port = 0;
        ::memcpy(&addr6->sin6_addr, addrByte, addrLen);
        addr6->sin6_family = AF_INET;
    }
    if (saFamily == AF_UNSPEC) {
        result = TCP::selectAddr(addrByte, port);
        ::memmove(addrByte + 1, addrByte, addrLen);
        addrByte[0] = (char) addrLen;
        addrLen++;
    }
    struct {
        uint8_t version;
        uint8_t code;
        /* must be 0x00 */
        uint8_t reserved;
        uint8_t addressType;
    } answer{};
    answer.version = SOCKS_VERSION;
    answer.code = 0x00;
    answer.reserved = 0;
    answer.addressType = header.addressType;
    sendRequestAnswer(client, &answer, addrByte, addrLen, port);
    return result;
}

void SocksServer::sendAnswer(const TCP::Socket& client, TCP::ipaddres adr) {
    struct {
        uint8_t version;
        uint8_t code;
        /* must be 0x00 */
        uint8_t reserved;
        uint8_t addressType;
    } answer{};
    answer.version = 5;
    answer.code = 0x00;
    answer.reserved = 0;
    answer.addressType = adr.family == AF_INET6 ? 0x04 : 0x01;
    client.writeSocket(&answer, sizeof(answer));
    auto* addr6 = (sockaddr_in6*) &adr.addr;
    auto* addrt = (sockaddr_in*) &adr.addr;
    switch (answer.addressType) {
        case 0x01:
        case 0x04:
            client.writeSocket(&addr6->sin6_addr, 16);
            break;
        case 0x03:
            client.writeSocket(&addrt->sin_addr, 4);
            break;
        default:
            std::cerr << "Wrong address type!\n";
            return;
    }
}


void SocksServer::accept(TCP::Socket socket , struct sockaddr addr) {
    std::cout << "Accept connection " << TCP::toString(addr) << "\n";
    int err = socket.setTimeOut(timeout);
    if (err != 0) {
        std::cerr << "Can't set socket timeout " << TCP::toString(addr) << strerror(err) << "\n";
        return;
    }
    if (!authorize(socket)) {
        std::cerr << "Authorize error " << TCP::toString(addr) << "\n";
        return;
    }
    std::cout << "Authorize! " << TCP::toString(addr) << "\n";
    TCP::ipaddres ipaddr = acceptCommand(socket);
    if (ipaddr.addrlen == 0) {
        std::cerr << "Can't accept connection " << TCP::toString(addr) << strerror(err) << "\n";
        return;
    }
    TCP::Socket server = TCP::connectSocket(ipaddr);
    SocksWorker worker(socket, server);
    worker.work();
}

 SocksServer::SocksServer(uint16_t port, struct timeval timeout) : TCP::Server(def_address, port), timeout(timeout) {}

class SocksConnection {

};