#pragma once
#include "Socket.h"

namespace TCP {

    class Server {
    protected:
        Socket _socket;
        virtual void accept(Socket, struct sockaddr) = 0;
    public:
        Server(const char* address, uint16_t port);

        void listen();

        Socket socket() {
            return _socket;
        }

    };

}
