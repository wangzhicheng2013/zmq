#include <iostream>
#include <chrono>
#include <thread>
#include "zmq_agent.hpp"

void server_thread() {
    zmq_self_agent::zmq_agent server;
    zmq_self_agent::zmq_config config;
    config.addr = "tcp://*:5555";
    config.sock_type = ZMQ_PULL;
    unsigned char ret = server.init(config);
    if (zmq_self_agent::NO_ERROR != ret) {
        std::cerr << "server init failed, error code = " << ret << std::endl;
        return;
    }
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::string str;
        server.recv(str);
        std::cout << "server recv = " << str << std::endl;
    }
}

void client_thread() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    zmq_self_agent::zmq_agent client;
    zmq_self_agent::zmq_config config;
    config.addr = "tcp://127.0.0.1:5555";
    config.sock_type = ZMQ_PUSH;
    unsigned char ret = client.init(config);
    if (zmq_self_agent::NO_ERROR != ret) {
        std::cerr << "client init failed error code = " << ret << std::endl;
        return;
    }
    std::string str = "hello zmq.";
    while (true) {
        std::cout << "client send size = " << client.send(str) << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
int main() {
    std::string str;
    zmq_self_agent::zmq_agent::get_version(str);
    std::cout << "zmq version = " << str << std::endl;

    std::thread server(server_thread);
    std::thread client(client_thread);
    if (server.joinable()) {
        server.join();
    }
    if (client.joinable()) {
        client.join();
    }
    
    return 0;
}