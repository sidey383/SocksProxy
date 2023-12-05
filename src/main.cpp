#include <iostream>
#include "../tcp/TcpSocket.h"


int main(int argc, char **argv) {
    if (argc < 1)
        return 0;
    char* address = argc < 2 ? nullptr : argv[1];
    TCP::Socket(address, (uint16_t) 8080);
}
