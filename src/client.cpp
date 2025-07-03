#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Format required: kv_client VERB [args]\n";
        return 1;
    }

    std::string message;
    for (int i = 1; i < argc; ++i) {
        message += argv[i];
        if (i + 1 < argc) {
            message += ' ';
        }
    }
    message += '\n';

    addrinfo hints{}, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo("127.0.0.1", "4000", &hints, &res);
    int fd = socket(res->ai_family, res->ai_socktype, 0);
    connect(fd, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);

    send(fd, message.c_str(), message.size(), 0);

    char buf[256];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        std::cout << buf;
    }
    close(fd);
    
    return 0;
}