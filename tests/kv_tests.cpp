#include <gtest/gtest.h>
#include "../src/kv_store.hpp"
#include <thread>
#include <netdb.h>
#include <signal.h>

TEST(KVStore, SingleThreadSetGetDel) {
    std::lock_guard<std::mutex> lock(m);
    map.clear();
    map["Hello"] = "World!";
    ASSERT_EQ(map["Hello"], "World!");
    map.erase("Hello");
    ASSERT_EQ(map.count("Hello"), 0);
}

TEST(KVStore, ConcurrencyTest) {
    {
        std::lock_guard<std::mutex> lock(m);
        map.clear();
    }
    
    const int n = 10000;
    auto insert_function = [](int thread_id) {
        for (int i = 0; i < n; ++i) {
            std::string k = "k" + std::to_string(thread_id * n + i);
            std::string v = "v" + std::to_string(i);
            {
                std::lock_guard<std::mutex> lock(m);
                map[k] = v;
            }
        }
    };

    std::vector<std::thread> threads;
    const int thread_count = 8;
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back(insert_function, i);
    }
    for (auto& thread : threads) {
        thread.join();
    }

    {
        std::lock_guard<std::mutex> lock(m);
        ASSERT_EQ(map.size(), thread_count * n);
    }
}

bool server_running(const std::string& host, const std::string& port) {
    addrinfo hints{}, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host.c_str(), port.c_str(), &hints, &res) != 0) return false;

    const int n = 100;
    for (int i = 0; i < n; ++i) {
        int fd = socket(res->ai_family, res->ai_socktype, 0);
        if (fd < 0) {
            freeaddrinfo(res);
            return false;
        }
        if(!connect(fd, res->ai_addr, res->ai_addrlen)) {
            close(fd);
            freeaddrinfo(res);
            return true;
        }
        close(fd);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    freeaddrinfo(res);
    return false;
}

std::string client_get(const std::string& host, const std::string& port, const std::string& key) {
    int pipe_fd[2];
    if (pipe(pipe_fd) != 0) {
        return "Pipe Error";
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return "Fork Error";
    }

    if (pid == 0) {
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        execl("./kv_client", host.c_str(), port.c_str(), "GET", key.c_str(), nullptr);
        exit(1);
    }

    close(pipe_fd[1]);
    std::string client_output;
    char buf[256];
    ssize_t bytes_read = read(pipe_fd[0], buf, sizeof(buf));
    while (bytes_read > 0) {
        client_output.append(buf, bytes_read);
        bytes_read = read(pipe_fd[0], buf, sizeof(buf));
    }

    close(pipe_fd[0]);
    int status;
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        return "Client failed";
    }

    return client_output;
}

void kill_server(pid_t pid) {
    kill(pid, SIGKILL);
    waitpid(pid, nullptr, 0);
}

TEST(KVStore, IntegrationTest) {
    std::string host = "127.0.0.1";
    std::string port = std::to_string(67876);
    
    pid_t pid = fork();
    ASSERT_GE(pid, 0) << "Fork error";

    if (pid == 0) {
        execl("./kv_server", "kv_server", port.c_str(), nullptr);
        exit(1);
    }

    if(!server_running(host, port)) {
        kill_server(pid);
        FAIL() << "Server error";
    }

    std::string client_call = "./kv_client " + host + " " + port;
    if(system((client_call + " SET Hello World!").c_str()) != 0) {
        kill_server(pid);
        FAIL() << "SET failed";
    }
    
    std::string client_output = client_get(host, port, "Hello");
    if(client_output != "$World!\n") {
        kill_server(pid);
        FAIL() << "GET failed with output: " << client_output;
    }

    if(system((client_call + " DEL Hello").c_str()) != 0) {
        kill_server(pid);
        FAIL() << "DEL failed";
    }

    client_output = client_get(host, port, "Hello");
    if(client_output != "NULL!\n") {
        kill_server(pid);
        FAIL() << "GET after DEL failed with output: " << client_output;
    }

    kill(pid, SIGINT);
    int status;
    waitpid(pid, &status, 0);
    ASSERT_TRUE(WIFEXITED(status));
}