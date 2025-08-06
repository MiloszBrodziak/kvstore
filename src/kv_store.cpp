#include "kv_store.hpp"

#include "ThreadPool.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <csignal>
#include <chrono>
#include <iostream>

int server_fd;
std::atomic<bool> running{true};
std::unordered_map<std::string, std::string> map;
std::mutex m;

void sigint_handler(int) { running = false; }

int open_listen_socket(const std::string& port) {
    addrinfo hints{}, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int s = getaddrinfo(nullptr, port.c_str(), &hints, &res);
    if (s != 0) throw std::runtime_error(gai_strerror(s));

    int fd = socket(res->ai_family, res->ai_socktype, 0);
    if (fd < 0) throw std::runtime_error("socket() failed");

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    if (bind(fd, res->ai_addr, res->ai_addrlen) != 0) throw std::runtime_error("bind() failed");
    freeaddrinfo(res);

    if (listen(fd, SOMAXCONN) != 0) throw std::runtime_error("listen() failed");
    return fd;
}

void handle_client(int client_fd) {
    auto start_time = std::chrono::high_resolution_clock::now();
    char buf[1024];
    ssize_t n = read(client_fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        std::string_view cmd(buf, n);
        if (cmd.back() == '\n' || cmd.back() == '\r') cmd.remove_suffix(1);
        std::string response;
        auto first_space = cmd.find(' ');
        std::string_view verb = cmd.substr(0, first_space);
        if (verb == "GET") {
            std::string key(cmd.substr(first_space + 1));
            {
                std::lock_guard lock(m);
                auto it = map.find(key);
                response = (it == map.end()) ? "NULL!\n" : "$" + it->second + "\n";
            }
        }
        else if (verb == "SET") {
            auto second_space = cmd.find(' ', first_space + 1);
            std::string key(cmd.substr(first_space + 1, second_space - first_space - 1));
            std::string val(cmd.substr(second_space + 1));
            {
                std::lock_guard lock(m);
                map[key] = val;
            }
            response = "SUCCESS!\n";
        }
        else if (verb == "DEL") {
            std::string key(cmd.substr(first_space + 1));
            {
                std::lock_guard lock(m);
                size_t num_removed = map.erase(key);
                response = num_removed ? "SUCCESS!\n" : "NULL!\n";
            }
        } else {
            response = "VERB COULD NOT BE PARSED!\n";
        }
        send(client_fd, response.c_str(), response.size(), MSG_NOSIGNAL);
    }
    close(client_fd);
    auto end_time = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    fprintf(stderr, "Latency: %lld microseconds\n", static_cast<long long>(time));
}

int main(int argc, char** argv) {
    try {
        struct sigaction sa{};
        sa.sa_handler = sigint_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGINT, &sa, nullptr);

        std::string port = (argc > 1) ? argv[1] : "4000";

        server_fd = open_listen_socket(port);
        std::cout << "Listening on Port:" << port << '\n';

        unsigned num_threads = std::thread::hardware_concurrency();
        if (num_threads == 0) num_threads = 4;
        ThreadPool tp(num_threads);

        while (running) {
            int client = accept(server_fd, nullptr, nullptr);
            if (client < 0) {
                if (errno == EINTR) break;
                perror("Accept Error");
                continue;
            }
            tp.enqueue([client] {handle_client(client);});
        }

        close(server_fd);
        std::cout << "\nServer Shutdown.\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Fatal Error: " << ex.what() << '\n';
        return 1;
    }
}