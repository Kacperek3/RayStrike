#pragma once

#include <iostream>
#include <thread>
#include <cstring> // memset
#include <unistd.h> // close
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h> // fcntl
#include <netinet/in.h>
#include <mutex>
#include <errno.h>
#include <string>

class NetworkGameManager {
public:
    NetworkGameManager(bool isServer, const std::string& ip, int port);
    ~NetworkGameManager();

    void SendPosition(float x, float y);
    bool IsServer() const { return _isServer; }
    void GetEnemyPosition(float& x, float& y);
private:
    std::thread _receiverThread;
    std::mutex _dataMutex;
    float _enemyX = 0.0f;
    float _enemyY = 0.0f;
    bool _isRunning;

    void ReceiveLoop();
    bool ReceivePosition(float& x, float& y);
    int _socket;
    int _clientSocket; // tylko dla serwera
    bool _isServer;
};
