#pragma once

#include <iostream>
#include <thread>
#include <cstring> // memset
#include <unistd.h> // close
#include <sys/socket.h>
#include <arpa/inet.h>

class NetworkGameManager {
public:
    NetworkGameManager::NetworkGameManager(bool isServer, const std::string& ip, int port);
    ~NetworkGameManager();

    void SendPosition(float x, float y);
    bool ReceivePosition(float& x, float& y);
    bool IsServer() const { return _isServer; }

private:
    int _socket;
    int _clientSocket; // tylko dla serwera
    bool _isServer;
};
