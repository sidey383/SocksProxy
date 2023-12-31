#include "SocksWorker.h"
#include <poll.h>
#include <functional>
#include <iostream>

bool SocksWorker::proxyClose(int in, int out) {
    std::cout << "Connection POLLHUP\n";
    return false;
}

bool SocksWorker::proxyWrongWal(int in, int out) {
    std::cout << "Error, wrong proxy file descriptor\n";
    return false;
}

bool SocksWorker::proxyError(int in, int out) {
    int err;
    socklen_t len = sizeof(err);
    int getError = getsockopt(in, SOL_SOCKET, SO_ERROR, &err, &len);
    if (getError == 0) {
        std::cout << "Connection error " << strerror(err) << "\n";
    } else {
        std::cout << "Connection error, can't get error " << strerror(errno) << "\n";
    }
    return false;
}

bool SocksWorker::proxyInPacket(int in, int out) {
    size_t paketSize = recv(in, _buffer, SOCKS_BUFFER_SIZE,   0);
    if (paketSize == -1) {
        std::cout << "Recv error "  << SOCKS_BUFFER_SIZE << "\n";
        return false;
    }

    size_t sendSize = 0;
    while (sendSize < paketSize) {
        size_t s = send(out, _buffer + sendSize, paketSize - sendSize, 0);
        if (s == -1) {
            std::cout << "Send error " << strerror(errno) << "\n";
            return false;
        }
        sendSize += s;
    }
    return true;
}

SocksWorker::~SocksWorker() {
    _client.close();
    _server.close();
}

void SocksWorker::work() {
    struct {
        std::function<bool(int, int)> func;
        short int flag;
    } actions[] = {
            {
                    [this](int in, int out) { return SocksWorker::proxyInPacket(in, out); },
                    POLLPRI | POLLIN
            },{
                    [this](int in, int out) { return SocksWorker::proxyClose(in, out); },
                    POLLHUP | POLLRDHUP
            },{
                    [this](int in, int out) { return SocksWorker::proxyWrongWal(in, out); },
                    POLLNVAL
            }, {
                    [this](int in, int out) { return SocksWorker::proxyError(in, out); },
                    POLLERR
            }
    };
    short int event = POLLIN | POLLPRI;
    pollfd polls[2] = {
            {_client.socket(), event, 0},
            {_server.socket(), event, 0},
    };
    bool isWork = true;
    while (isWork) {
        int status = ::poll(polls, 2, 10000);
        if (status == -1) {
            std::cerr << "Poll error\n";
            break;
        }
        std::cout << "Poll " << status << " " << polls[0].revents << " " << polls[1].revents << "\n";
        for (int i = 0; i < 2; i++) {
            pollfd in = polls[i];
            pollfd out = polls[ (i + 1)%2];
            for (auto & action : actions) {
                if ((action.flag & in.revents) != 0) {
                    in.revents = (short) (in.revents & (~action.flag));
                    isWork = action.func(in.fd, out.fd);
                    if (!isWork)
                        break;
                }
            }
            if (!isWork)
                break;
            if (in.revents != 0) {
                std::cerr << "Not catched events " << in.revents << "\n";
                in.revents = 0;
            }
            int err;
            socklen_t len = sizeof(err);
            int getError = getsockopt(in.fd, SOL_SOCKET, SO_ERROR, &err, &len);
            if (getError != 0) {
                std::cout << "Connection error, can't get error " << strerror(errno) << "\n";
                isWork = false;
                break;
            }
            if (err != 0) {
                std::cout << "Connection error " << strerror(err) << "\n";
                isWork = false;
                break;
            }
        }
    }
    std::cout << "Close connection"<< "\n";
}
