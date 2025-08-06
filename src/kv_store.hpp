#pragma once
#include <mutex>
#include <string>
#include <unordered_map>
#include <atomic>

extern int server_fd;
extern std::atomic<bool> running;
extern std::unordered_map<std::string, std::string> map;
extern std::mutex m;

void sigint_handler(int);
int open_listen_socket(const std::string& port);
void handle_client(int client_fd);